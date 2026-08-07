#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0

import io
import os
import re
import sys

import numpy as np
import streamlit as st
from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from tilecanvas import tile_canvas

TILE = 8
GRASS = (211, 207, 178)
TOL = 40
SCREEN_W = 256
SCREEN_H = 192
MAX_WORLD = 2048

SOLID_BASE = (101, 67, 33)
SOLID_GRID = (70, 45, 20)

WALL = 1
TREE = 2
BUSH = 3
SHALLOW_WATER = 4
DEEP_WATER = 5
FIRE = 6

FILL = {WALL: 255, TREE: 210, BUSH: 170, SHALLOW_WATER: 130, DEEP_WATER: 85, FIRE: 40}

COLORS = {
    WALL: (101, 67, 33, 217),
    TREE: (34, 100, 34, 217),
    BUSH: (80, 150, 60, 200),
    SHALLOW_WATER: (100, 180, 255, 190),
    DEEP_WATER: (40, 100, 220, 217),
    FIRE: (255, 60, 60, 230),
}

TOOLS = [
    ("Wall (brown)", WALL),
    ("Tree (dark green)", TREE),
    ("Bush (green)", BUSH),
    ("Shallow water (light blue)", SHALLOW_WATER),
    ("Deep water (dark blue)", DEEP_WATER),
    ("Fire (orange)", FIRE),
    ("Erase", 0),
]


def repo_root():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def snap8(v):
    return (v + TILE - 1) // TILE * TILE


def solid_block(w, h):
    img = Image.new("RGBA", (w, h), SOLID_BASE + (255,))
    d = ImageDraw.Draw(img)
    for x in range(0, w + 1, TILE):
        d.line([(x, 0), (x, h)], fill=SOLID_GRID + (255,), width=1)
    for y in range(0, h + 1, TILE):
        d.line([(0, y), (w, y)], fill=SOLID_GRID + (255,), width=1)
    return img


def detect_state(img):
    px = np.asarray(img.convert("RGB"))
    ph = px.shape[0] // TILE
    pw = px.shape[1] // TILE
    state = np.zeros((ph, pw), dtype=np.uint8)
    for ty in range(ph):
        for tx in range(pw):
            block = px[ty * TILE:(ty + 1) * TILE, tx * TILE:(tx + 1) * TILE]
            r = block[..., 0].astype(int)
            g = block[..., 1].astype(int)
            b = block[..., 2].astype(int)
            nongrass = ~((np.abs(r - GRASS[0]) <= TOL) &
                         (np.abs(g - GRASS[1]) <= TOL) &
                         (np.abs(b - GRASS[2]) <= TOL))
            if (b - r >= 30).mean() >= 0.5:
                state[ty, tx] = DEEP_WATER
            elif nongrass.mean() >= 0.5:
                rm = r[nongrass].mean()
                gm = g[nongrass].mean()
                bm = b[nongrass].mean()
                if gm > rm + 20 and gm > bm + 20:
                    if gm < 100:
                        state[ty, tx] = TREE
                    else:
                        state[ty, tx] = BUSH
                else:
                    state[ty, tx] = WALL
    return state


def prepare_canvas(art, w, h):
    img = art.convert("RGBA")
    note = []
    aw = min(img.width, w)
    ah = min(img.height, h)
    if img.width > w or img.height > h:
        img = img.crop((0, 0, aw, ah))
        note.append("cropped (art larger than world)")
    if img.width < w or img.height < h:
        pad = solid_block(w, h)
        pad.paste(img, (0, 0))
        img = pad
        note.append("padded with solid blocks")
    return img, note, (aw, ah)


def new_grid(ph, pw, aw, ah):
    grid = np.zeros((ph, pw), dtype=np.uint8)
    for tx in range(pw):
        if tx * TILE >= aw:
            grid[:, tx] = WALL
    for ty in range(ph):
        if ty * TILE >= ah:
            grid[ty, :] = WALL
    return grid


def canvas_scale(w, h):
    return 2 if max(w, h) <= 512 else 1


def grid_background(img, scale):
    w = img.width * scale
    h = img.height * scale
    bg = img.resize((w, h), Image.NEAREST)
    ov = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(ov)
    step = TILE * scale
    for x in range(0, w + 1, step):
        d.line([(x, 0), (x, h)], fill=(200, 200, 200, 160), width=1)
    for y in range(0, h + 1, step):
        d.line([(0, y), (w, y)], fill=(200, 200, 200, 160), width=1)
    return Image.alpha_composite(bg, ov)


def state_to_mask(state, scale):
    ts = TILE * scale
    w = state.shape[1] * ts
    h = state.shape[0] * ts
    arr = np.zeros((h, w, 4), dtype=np.uint8)
    for ty in range(state.shape[0]):
        for tx in range(state.shape[1]):
            v = int(state[ty, tx])
            if v:
                arr[ty * ts:(ty + 1) * ts, tx * ts:(tx + 1) * ts] = COLORS[v]
    return arr


def build_colmap(state, w, h):
    pw = w // TILE
    ph = h // TILE
    level = np.zeros((ph, pw), dtype=np.uint8)
    sw = min(pw, state.shape[1])
    sh = min(ph, state.shape[0])
    level[:sh, :sw] = state[:sh, :sw]
    out = Image.new("L", (w, h + TILE), 0)
    d = ImageDraw.Draw(out)
    for idx, fill in enumerate(FILL.values(), start=1):
        d.rectangle([idx * TILE, 0, idx * TILE + TILE - 1, TILE - 1], fill=fill)
    for ty in range(ph):
        for tx in range(pw):
            fill = FILL.get(int(level[ty, tx]))
            if fill:
                x0 = tx * TILE
                y0 = (ty + 1) * TILE
                d.rectangle([x0, y0, x0 + TILE - 1, y0 + TILE - 1], fill=fill)
    buf = io.BytesIO()
    out.save(buf, format="PNG")
    return buf.getvalue()


def sync_level_h(w, h):
    path = os.path.join(repo_root(), "source", "level.h")
    try:
        with open(path) as f:
            text = f.read()
    except OSError:
        return False
    changed = False
    out = []
    for line in text.splitlines(keepends=True):
        m = re.match(r"\s*#define\s+(LEVEL_[WH])\s+(\d+)\s*$", line)
        if not m:
            out.append(line)
            continue
        val = w if m.group(1) == "LEVEL_W" else h
        repl = re.sub(r"\d+", str(val), line, count=1)
        if repl != line:
            changed = True
        out.append(repl)
    if changed:
        with open(path, "w") as f:
            f.write("".join(out))
    return changed


def main():
    st.set_page_config(page_title="Chilling Mech — collision painter",
                       layout="wide")
    st.title("Collision map painter")

    art = None
    uploaded = st.sidebar.file_uploader("Load image", type=["png", "jpg"])
    if uploaded is not None:
        art = Image.open(uploaded)
        st.sidebar.caption("Loaded from upload")
    else:
        path = os.path.join(repo_root(), "assets", "forest.png")
        if os.path.exists(path):
            art = Image.open(path)
            st.sidebar.caption("Default: assets/forest.png")
        else:
            st.sidebar.error("No image. Upload one or restore forest.png.")

    if art is None:
        st.stop()

    min_w = max(SCREEN_W, snap8(min(art.width, MAX_WORLD)))
    min_h = max(SCREEN_H, snap8(min(art.height, MAX_WORLD)))
    world_w = st.sidebar.number_input("World width (px)", min_value=min_w,
                                      max_value=MAX_WORLD, step=TILE,
                                      value=min_w)
    world_h = st.sidebar.number_input("World height (px)", min_value=min_h,
                                      max_value=MAX_WORLD, step=TILE,
                                      value=min_h)
    world_w = min(MAX_WORLD, snap8(world_w))
    world_h = min(MAX_WORLD, snap8(world_h))

    canvas_img, notes, (aw, ah) = prepare_canvas(art, world_w, world_h)
    if notes:
        st.sidebar.caption("Art %dx%d → %s" % (art.width, art.height,
                                               ", ".join(notes)))

    pw = world_w // TILE
    ph = world_h // TILE
    bg_w = ((world_w + 255) // 256) * 256
    bg_h = ((world_h + 255) // 256) * 256
    scale = canvas_scale(world_w, world_h)
    st.sidebar.write("World: %dx%d px = %dx%d tiles" % (world_w, world_h, pw,
                                                        ph))
    if (bg_w, bg_h) != (world_w, world_h):
        st.sidebar.caption("DS bg padded to %dx%d (multiple of 256)" %
                           (bg_w, bg_h))

    if "grid" not in st.session_state:
        st.session_state.grid = None
    if "world_size" not in st.session_state:
        st.session_state.world_size = (0, 0)
    if "rev" not in st.session_state:
        st.session_state.rev = 0
    if st.session_state.world_size != (world_w, world_h):
        st.session_state.grid = new_grid(ph, pw, aw, ah)
        st.session_state.world_size = (world_w, world_h)
        st.session_state.rev += 1
    if (st.session_state.grid is None
            or st.session_state.grid.shape != (ph, pw)):
        st.session_state.grid = new_grid(ph, pw, aw, ah)
        st.session_state.rev += 1

    mode = st.sidebar.radio("Tool", [t[0] for t in TOOLS])
    paint_value = dict(TOOLS)[mode]

    if st.sidebar.button("Detect (auto walls, trees, bushes, water)"):
        st.session_state.grid = detect_state(canvas_img)
        st.session_state.rev += 1
    if st.sidebar.button("Clear"):
        st.session_state.grid = new_grid(ph, pw, aw, ah)
        st.session_state.rev += 1

    result = tile_canvas(
        background=grid_background(canvas_img, scale),
        grid=st.session_state.grid,
        cols=pw,
        rows=ph,
        tile=TILE * scale,
        paint_value=paint_value,
        rev=st.session_state.rev,
        key="tilecanvas",
    )
    if result is not None:
        arr = np.asarray(result.get("grid", []), dtype=np.uint8)
        if arr.size == ph * pw:
            painted = arr.reshape(ph, pw)
            if not np.array_equal(painted, st.session_state.grid):
                st.session_state.grid = painted

    grid = st.session_state.grid
    counts = {v: int((grid == v).sum()) for v in (WALL, TREE, BUSH, SHALLOW_WATER, DEEP_WATER, FIRE)}
    colmap = build_colmap(grid, world_w, world_h)

    preview = grid_background(canvas_img, scale)
    preview = Image.alpha_composite(preview,
                                    Image.fromarray(state_to_mask(grid, scale)))

    c1, c2 = st.columns(2)
    c1.write("Painted: wall %d | tree %d | bush %d | shallow %d | deep %d | fire %d" %
             (counts[WALL], counts[TREE], counts[BUSH], counts[SHALLOW_WATER], counts[DEEP_WATER], counts[FIRE]))
    c1.image(preview, use_column_width=True)
    c2.write("Exported collision map (%dx%d)" % (world_w, world_h + TILE))
    c2.image(colmap, use_column_width=True)

    out_path = os.path.join(repo_root(), "assets", "colmap.png")
    if st.button("Save to %s" % out_path):
        with open(out_path, "wb") as f:
            f.write(colmap)
        msg = "Saved %s (%dx%d tiles)" % (out_path, pw, ph)
        if sync_level_h(world_w, world_h):
            msg += " | synced source/level.h to %dx%d" % (world_w, world_h)
        st.success(msg)
    st.download_button("Download colmap.png", colmap,
                       file_name="colmap.png", mime="image/png")


if __name__ == "__main__":
    main()
