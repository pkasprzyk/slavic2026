#!/usr/bin/env python3
"""Generate forest.png, colmap.png and spawn.dat from assets/tiles/project.tmx."""

import os
import sys
import xml.etree.ElementTree as ET

from PIL import Image

TILE = 8

WALL = 1
TREE = 2
BUSH = 3
SHALLOW_WATER = 4
DEEP_WATER = 5
FIRE_TYPE = 6

FILL = {
    WALL: 255,
    TREE: 210,
    BUSH: 170,
    SHALLOW_WATER: 130,
    DEEP_WATER: 85,
    FIRE_TYPE: 40,
}

FOREST_LAYERS = ["floor", "deep_water", "sides", "shallow_water", "bushes", "big_trees"]
COLMAP_LAYERS = FOREST_LAYERS + ["fire"]

LAYER_COLLISION = {
    "sides": WALL,
    "deep_water": DEEP_WATER,
    "shallow_water": SHALLOW_WATER,
    "bushes": BUSH,
    "big_trees": TREE,
    "fire": FIRE_TYPE,
}


def parse_tmx(tmx_path):
    tree = ET.parse(tmx_path)
    root = tree.getroot()
    map_w = int(root.attrib["width"])
    map_h = int(root.attrib["height"])
    firstgid = int(root.find("tileset").attrib["firstgid"])
    tsx_source = root.find("tileset").attrib["source"]

    layers = {}
    for layer_el in root.findall("layer"):
        name = layer_el.attrib["name"]
        data_el = layer_el.find("data")
        encoding = data_el.attrib.get("encoding", "csv")
        if encoding != "csv":
            print(f"ERROR: layer '{name}' encoding is '{encoding}', only csv supported",
                  file=sys.stderr)
            sys.exit(1)
        text = data_el.text.strip()
        rows = [r.strip() for r in text.split("\n") if r.strip()]
        grid = [[0] * map_w for _ in range(map_h)]
        for y, row in enumerate(rows):
            vals = [v.strip() for v in row.split(",") if v.strip() != ""]
            for x, v in enumerate(vals):
                grid[y][x] = int(v)
        layers[name] = grid

    return layers, map_w, map_h, firstgid, tsx_source


def parse_spawns(tmx_path):
    tree = ET.parse(tmx_path)
    root = tree.getroot()
    player = None
    bunnies = []

    for og_el in root.findall("objectgroup"):
        name = og_el.attrib.get("name", "")
        for obj_el in og_el.findall("object"):
            obj_type = obj_el.attrib.get("type", "")
            x = int(float(obj_el.attrib["x"]))
            y = int(float(obj_el.attrib["y"]))
            if name == "player" and obj_type == "player":
                player = (x, y)
            elif obj_type == "bunny":
                bunnies.append((x, y))

    if player is None:
        for og_el in root.findall("objectgroup"):
            for obj_el in og_el.findall("object"):
                if obj_el.attrib.get("type", "") == "player":
                    x = int(float(obj_el.attrib["x"]))
                    y = int(float(obj_el.attrib["y"]))
                    player = (x, y)
                    break
            if player is not None:
                break

    if player is None:
        print("ERROR: no 'player' object found", file=sys.stderr)
        sys.exit(1)
    return player, bunnies


def write_spawn_binary(player, bunnies, out_path):
    import struct
    with open(out_path, "wb") as f:
        f.write(struct.pack("<HH", player[0], player[1]))
        f.write(struct.pack("<B", len(bunnies)))
        for x, y in bunnies:
            f.write(struct.pack("<HH", x, y))


def parse_tsx(tsx_path):
    tree = ET.parse(tsx_path)
    root = tree.getroot()
    tsx_dir = os.path.dirname(tsx_path)
    tiles = {}
    for tile_el in root.findall("tile"):
        tid = int(tile_el.attrib["id"])
        img_el = tile_el.find("image")
        source = img_el.attrib["source"]
        w = int(img_el.attrib["width"])
        h = int(img_el.attrib["height"])
        img_path = os.path.join(tsx_dir, source)
        tiles[tid] = (img_path, w, h)
    return tiles


def load_tile_images(tiles, firstgid):
    images = {}
    for tid, (path, w, h) in tiles.items():
        gid = tid + firstgid
        img = Image.open(path).convert("RGBA")
        images[gid] = (img, w, h)
    return images


def apply_gid_flags(image, gid):
    flags = gid & 0xE0000000
    img = image.copy()
    if flags & 0x80000000:
        img = img.transpose(Image.FLIP_LEFT_RIGHT)
    if flags & 0x40000000:
        img = img.transpose(Image.FLIP_TOP_BOTTOM)
    if flags & 0x20000000:
        img = img.transpose(Image.ROTATE_90)
        img = img.transpose(Image.FLIP_LEFT_RIGHT)
    return img


def composite_forest(layers, tile_images, map_w, map_h):
    pw = map_w * TILE
    ph = map_h * TILE
    canvas = Image.new("RGBA", (pw, ph), (0, 0, 0, 0))

    for layer_name in FOREST_LAYERS:
        grid = layers.get(layer_name)
        if grid is None:
            continue
        for ty in range(map_h):
            for tx in range(map_w):
                raw_gid = grid[ty][tx]
                if raw_gid == 0:
                    continue
                base_gid = raw_gid & 0x1FFFFFFF
                entry = tile_images.get(base_gid)
                if entry is None:
                    continue
                tile_img, tw, th = entry
                x = tx * TILE
                y = (ty + 1) * TILE - th
                if tw != TILE or th != TILE:
                    final_w = tw
                    final_h = th
                else:
                    final_w = TILE
                    final_h = TILE
                rendered = apply_gid_flags(tile_img, raw_gid)
                canvas.paste(rendered, (x, y), rendered)
    return canvas


def build_colmap(layers, tile_images, map_w, map_h):
    pw = map_w * TILE
    ph = (map_h + 1) * TILE
    canvas = Image.new("L", (pw, ph), 0)
    col_grid = [[0] * map_w for _ in range(map_h)]

    for layer_name in COLMAP_LAYERS:
        col_type = LAYER_COLLISION.get(layer_name)
        if col_type is None:
            continue
        grid = layers.get(layer_name)
        if grid is None:
            continue
        for ty in range(map_h):
            for tx in range(map_w):
                raw_gid = grid[ty][tx]
                if raw_gid == 0:
                    continue
                base_gid = raw_gid & 0x1FFFFFFF
                entry = tile_images.get(base_gid)
                if entry is None:
                    continue
                _, tw, th = entry
                if tw != TILE or th != TILE:
                    cells_w = tw // TILE
                    cells_h = th // TILE
                    for dy in range(cells_h):
                        for dx in range(cells_w):
                            cy = ty - dy
                            cx = tx + dx
                            if 0 <= cy < map_h and 0 <= cx < map_w:
                                col_grid[cy][cx] = col_type
                else:
                    col_grid[ty][tx] = col_type

    for idx, (ctype, fill) in enumerate(FILL.items(), start=1):
        x0 = idx * TILE
        y0 = 0
        for py in range(TILE):
            for px in range(TILE):
                canvas.putpixel((x0 + px, y0 + py), fill)

    for ty in range(map_h):
        for tx in range(map_w):
            ctype = col_grid[ty][tx]
            fill = FILL.get(ctype)
            if fill is None:
                continue
            x0 = tx * TILE
            y0 = (ty + 1) * TILE
            for py in range(TILE):
                for px in range(TILE):
                    canvas.putpixel((x0 + px, y0 + py), fill)

    return canvas


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    tmx_path = os.path.join(script_dir, "tiles", "project.tmx")
    out_forest = os.path.join(script_dir, "forest.png")
    out_colmap = os.path.join(script_dir, "colmap.png")
    out_spawn = os.path.join(script_dir, "spawn.dat")

    player, bunnies = parse_spawns(tmx_path)
    write_spawn_binary(player, bunnies, out_spawn)
    print(f"Wrote {out_spawn} (player=({player[0]},{player[1]}), {len(bunnies)} bunnies)")

    layers, map_w, map_h, firstgid, tsx_source = parse_tmx(tmx_path)
    tsx_path = os.path.join(os.path.dirname(tmx_path), tsx_source)
    tiles = parse_tsx(tsx_path)
    tile_images = load_tile_images(tiles, firstgid)

    forest = composite_forest(layers, tile_images, map_w, map_h)
    forest.save(out_forest)
    print(f"Wrote {out_forest} ({forest.width}x{forest.height})")

    colmap = build_colmap(layers, tile_images, map_w, map_h)
    colmap.save(out_colmap)
    print(f"Wrote {out_colmap} ({colmap.width}x{colmap.height})")


if __name__ == "__main__":
    main()
