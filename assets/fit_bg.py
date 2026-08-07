#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
# Crop/pad an image to a target size for GRIT (called by assets/convert.sh).

import sys

from PIL import Image


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
            bg = Image.new("L", (w, h), 0)
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
