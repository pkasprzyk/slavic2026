#!/bin/bash -xeu

BLOCKSDS="${BLOCKSDS:-/opt/wonderful/thirdparty/blocksds/core/}"
GRIT=$BLOCKSDS/tools/grit/grit

mkdir -p ../nitrofiles/spr ../nitrofiles/bg ../nitrofiles/fnt ../nitrofiles/maps ../nitrofiles/audio

$GRIT mech.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

mv *.pal *.img ../nitrofiles/spr

# compile the font
$GRIT default.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

mv *.pal *.img ../nitrofiles/fnt

LEVEL_W=$(sed -n 's/.*#define LEVEL_W[[:space:]]*\([0-9][0-9]*\).*/\1/p' \
          ../source/level.h | head -1)
LEVEL_H=$(sed -n 's/.*#define LEVEL_H[[:space:]]*\([0-9][0-9]*\).*/\1/p' \
          ../source/level.h | head -1)
BG_W=$(( ((LEVEL_W + 255) / 256) * 256 ))
BG_H=$(( ((LEVEL_H + 255) / 256) * 256 ))

FIT_OK=0
if [ -n "$LEVEL_W" ] && [ -n "$LEVEL_H" ] &&
   command -v python3 >/dev/null 2>&1 &&
   python3 -c 'import PIL' >/dev/null 2>&1; then
    FIT_OK=1
else
    echo "WARN: python3+PIL missing or source/level.h unreadable; " \
         "bg/colmap may not match world size" >&2
fi

if [ "$FIT_OK" = "1" ]; then
    TMPD=$(mktemp -d)
    trap 'rm -rf "$TMPD"' EXIT
    python3 fit_bg.py forest.png "$TMPD/forest.png" \
        "$BG_W" "$BG_H" solid
    BG_SRC="$TMPD/forest.png"
else
    BG_SRC=forest.png
fi

$GRIT "$BG_SRC" -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs

mv *.pal *.img *.map ../nitrofiles/bg

if [ "$FIT_OK" = "1" ]; then
    python3 fit_bg.py colmap.png "$TMPD/colmap.png" \
        "$LEVEL_W" "$((LEVEL_H + 8))" solid
    COL_SRC="$TMPD/colmap.png"
else
    COL_SRC=colmap.png
fi

$GRIT "$COL_SRC" -ftB -fh! -g! -gB8 -mRt -mLf -p!

mv *.map ../nitrofiles/maps

cp *.wav ../nitrofiles/audio
