#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
# Crop/pad a background image to a target size for GRIT.
#
# Subcommands:
#
#   fit   Fit a single image to an explicit size:
#             fit_bg.py fit IN OUT W H [--pad PAD]
#
#   all   Fit the project background assets (forest.png, colmap.png) using the
#         level size from source/level.h. forest.png is padded to the next
#         multiple of 256, colmap.png to LEVEL_W x (LEVEL_H + 8):
#             fit_bg.py all [--outdir DIR] [--assets DIR] [--level PATH]
#                           [--pad PAD]
#
# PAD is "solid" (solid-block wall pattern with 8x8 tile grid, or
# collision-solid in L-mode maps) or a hex color like "D3CFB2".

import argparse
import os
import re
import sys

from PIL import Image, ImageDraw

TILE = 8
SOLID_BASE = (101, 67, 33)
SOLID_GRID = (70, 45, 20)
SOLID_COLL = 255


def solid_block(w, h):
    img = Image.new("RGBA", (w, h), SOLID_BASE + (255,))
    d = ImageDraw.Draw(img)
    for x in range(0, w + 1, TILE):
        d.line([(x, 0), (x, h)], fill=SOLID_GRID + (255,), width=1)
    for y in range(0, h + 1, TILE):
        d.line([(0, y), (w, y)], fill=SOLID_GRID + (255,), width=1)
    return img


def fit(src, dst, w, h, pad):
    img = Image.open(src)
    if img.mode != "L":
        img = img.convert("RGBA")

    if img.width > w or img.height > h:
        img = img.crop((0, 0, min(img.width, w), min(img.height, h)))

    if img.width < w or img.height < h:
        solid = pad.lower() == "solid"
        if img.mode == "L":
            bg = Image.new("L", (w, h), SOLID_COLL if solid else int(pad, 16))
        elif solid:
            bg = solid_block(w, h)
        else:
            rgb = tuple(int(pad[i:i + 2], 16) for i in (0, 2, 4))
            bg = Image.new("RGBA", (w, h), rgb + (255,))
        bg.paste(img, (0, 0))
        img = bg

    img.save(dst)
    print("fit: %s (%dx%d) -> %s (%dx%d)" % (src, w, h, dst, img.width,
                                             img.height))


def read_level(path):
    w = h = None
    try:
        with open(path) as f:
            for line in f:
                m = re.match(r"\s*#define\s+LEVEL_([WH])\s+(\d+)\s*$", line)
                if m:
                    if m.group(1) == "W":
                        w = int(m.group(2))
                    else:
                        h = int(m.group(2))
    except OSError:
        pass
    return w, h


def cmd_all(args):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    assets = os.path.abspath(args.assets or script_dir)
    level = os.path.abspath(args.level or os.path.join(script_dir, "..",
                                                       "source", "level.h"))
    outdir = os.path.abspath(args.outdir or os.path.join(assets, "fitted"))
    w, h = read_level(level)
    if w is None or h is None:
        print("fit_bg: could not read LEVEL_W/LEVEL_H from %s" % level,
              file=sys.stderr)
        return 1
    os.makedirs(outdir, exist_ok=True)
    bg_w = ((w + 255) // 256) * 256
    bg_h = ((h + 255) // 256) * 256
    fit(os.path.join(assets, "forest.png"), os.path.join(outdir, "forest.png"),
        bg_w, bg_h, args.pad)
    fit(os.path.join(assets, "colmap.png"), os.path.join(outdir, "colmap.png"),
        w, h + TILE, args.pad)
    print("fit_bg: fitted to %s" % outdir)
    return 0


def main():
    parser = argparse.ArgumentParser(
        description="Crop/pad background images to a target size for GRIT.")
    sub = parser.add_subparsers(dest="cmd", required=True)

    p_fit = sub.add_parser("fit", help="fit a single image")
    p_fit.add_argument("src")
    p_fit.add_argument("dst")
    p_fit.add_argument("width", type=int)
    p_fit.add_argument("height", type=int)
    p_fit.add_argument("--pad", default="solid",
                       help='padding: "solid" (8x8 tile grid) or hex color '
                            '(default: solid)')

    p_all = sub.add_parser("all",
                           help="fit project assets from source/level.h")
    p_all.add_argument("--outdir",
                       help="output directory (default: assets/fitted)")
    p_all.add_argument("--assets", help="assets directory (default: script dir)")
    p_all.add_argument("--level", help="path to source/level.h")
    p_all.add_argument("--pad", default="solid",
                       help='padding: "solid" (8x8 tile grid) or hex color '
                            '(default: solid)')

    args = parser.parse_args()
    if args.cmd == "fit":
        fit(args.src, args.dst, args.width, args.height, args.pad)
        return 0
    return cmd_all(args)


if __name__ == "__main__":
    sys.exit(main())
