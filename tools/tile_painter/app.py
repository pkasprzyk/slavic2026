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
MIN_WORLD = 256
MAX_WORLD = 2048
STEP = 256

SOLID = 1
WATER = 2
FIRE = 3

FILL = {SOLID: 255, WATER: 200, FIRE: 128}

COLORS = {
    SOLID: (101, 67, 33, 217),
    WATER: (80, 140, 255, 217),
    FIRE: (255, 60, 60, 230),
}

TOOLS = [
    ("Solid (red)", SOLID),
    ("Water (blue)", WATER),
    ("Fire (orange)", FIRE),
    ("Erase", 0),
]


def repo_root():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def snap(v):
    return min(MAX_WORLD, max(MIN_WORLD, ((v + STEP - 1) // STEP) * STEP))


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
                state[ty, tx] = WATER
            elif nongrass.mean() >= 0.5:
                state[ty, tx] = SOLID
    return state


def prepare_canvas(art, w, h):
    img = art.convert("RGBA")
    note = []
    if img.width > w or img.height > h:
        img = img.crop((0, 0, min(img.width, w), min(img.height, h)))
        note.append("cropped")
    if img.width < w or img.height < h:
        pad = Image.new("RGBA", (w, h), GRASS + (255,))
        pad.paste(img, (0, 0))
        img = pad
        note.append("padded with floor color")
    return img, note


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

    def_w = art.width if (art.width % STEP == 0 and art.width >= MIN_WORLD
                          and art.width <= MAX_WORLD) else MIN_WORLD
    def_h = art.height if (art.height % STEP == 0 and art.height >= MIN_WORLD
                           and art.height <= MAX_WORLD) else MIN_WORLD
    world_w = st.sidebar.number_input("World width (px)", min_value=MIN_WORLD,
                                      max_value=MAX_WORLD, step=STEP,
                                      value=def_w)
    world_h = st.sidebar.number_input("World height (px)", min_value=MIN_WORLD,
                                      max_value=MAX_WORLD, step=STEP,
                                      value=def_h)
    world_w = snap(world_w)
    world_h = snap(world_h)
    if world_w != def_w or world_h != def_h:
        st.sidebar.caption("Snapped to multiples of %d: %dx%d" %
                           (STEP, world_w, world_h))

    canvas_img, notes = prepare_canvas(art, world_w, world_h)
    if notes:
        st.sidebar.caption("Art %dx%d → %s" % (art.width, art.height,
                                               ", ".join(notes)))

    pw = world_w // TILE
    ph = world_h // TILE
    scale = canvas_scale(world_w, world_h)
    st.sidebar.write("World: %dx%d px = %dx%d tiles" % (world_w, world_h, pw,
                                                        ph))

    if "grid" not in st.session_state:
        st.session_state.grid = np.zeros((ph, pw), dtype=np.uint8)
    if "world_size" not in st.session_state:
        st.session_state.world_size = (world_w, world_h)
    if "rev" not in st.session_state:
        st.session_state.rev = 0
    if st.session_state.world_size != (world_w, world_h):
        st.session_state.grid = np.zeros((ph, pw), dtype=np.uint8)
        st.session_state.world_size = (world_w, world_h)
        st.session_state.rev += 1
    if st.session_state.grid.shape != (ph, pw):
        st.session_state.grid = np.zeros((ph, pw), dtype=np.uint8)
        st.session_state.rev += 1

    mode = st.sidebar.radio("Tool", [t[0] for t in TOOLS])
    paint_value = dict(TOOLS)[mode]

    if st.sidebar.button("Detect (auto solid + water)"):
        st.session_state.grid = detect_state(canvas_img)
        st.session_state.rev += 1
    if st.sidebar.button("Clear"):
        st.session_state.grid = np.zeros((ph, pw), dtype=np.uint8)
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
            new_grid = arr.reshape(ph, pw)
            if not np.array_equal(new_grid, st.session_state.grid):
                st.session_state.grid = new_grid

    grid = st.session_state.grid
    counts = {v: int((grid == v).sum()) for v in (SOLID, WATER, FIRE)}
    colmap = build_colmap(grid, world_w, world_h)

    preview = grid_background(canvas_img, scale)
    preview = Image.alpha_composite(preview,
                                    Image.fromarray(state_to_mask(grid, scale)))

    c1, c2 = st.columns(2)
    c1.write("Painted: solid %d | water %d | fire %d" %
             (counts[SOLID], counts[WATER], counts[FIRE]))
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
