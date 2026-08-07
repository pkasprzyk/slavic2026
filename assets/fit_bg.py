#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
# Crop/pad an image to a target size for GRIT (called by assets/convert.sh).
# Pad value: a hex color like "D3CFB2", or "solid" for a solid-block wall
# pattern (used to pad backgrounds to a multiple of 256).

import sys

from PIL import Image, ImageDraw

TILE = 8
SOLID_BASE = (101, 67, 33)
SOLID_GRID = (70, 45, 20)


def solid_block(w, h):
    img = Image.new("RGBA", (w, h), SOLID_BASE + (255,))
    d = ImageDraw.Draw(img)
    for x in range(0, w + 1, TILE):
        d.line([(x, 0), (x, h)], fill=SOLID_GRID + (255,), width=1)
    for y in range(0, h + 1, TILE):
        d.line([(0, y), (w, y)], fill=SOLID_GRID + (255,), width=1)
    return img


def main():
    if len(sys.argv) != 6:
        print("usage: fit_bg.py IN OUT W H PADHEX", file=sys.stderr)
        return 2
    src, dst = sys.argv[1], sys.argv[2]
    w, h = int(sys.argv[3]), int(sys.argv[4])
    pad = sys.argv[5]

    img = Image.open(src)
    if img.mode != "L":
        img = img.convert("RGBA")

    if img.width > w or img.height > h:
        img = img.crop((0, 0, min(img.width, w), min(img.height, h)))

    if img.width < w or img.height < h:
        if img.mode == "L":
            fill = 255 if pad.lower() == "solid" else int(pad, 16)
            bg = Image.new("L", (w, h), fill)
        elif pad.lower() == "solid":
            bg = solid_block(w, h)
        else:
            rgb = tuple(int(pad[i:i + 2], 16) for i in (0, 2, 4))
            bg = Image.new("RGBA", (w, h), rgb + (255,))
        bg.paste(img, (0, 0))
        img = bg

    img.save(dst)
    print("fit: %s (%dx%d) -> %s (%dx%d)" % (src, w, h, dst, img.width,
                                              img.height))
    return 0


if __name__ == "__main__":
    sys.exit(main())
