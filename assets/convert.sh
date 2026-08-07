#!/bin/bash -xeu

BLOCKSDS="${BLOCKSDS:-/opt/wonderful/thirdparty/blocksds/core/}"
GRIT=$BLOCKSDS/tools/grit/grit

mkdir -p ../nitrofiles/spr ../nitrofiles/bg ../nitrofiles/fnt ../nitrofiles/maps ../nitrofiles/audio

$GRIT mech.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT sprite_robot_idle.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT sprite_hare_idle.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

mv *.pal *.img ../nitrofiles/spr

# compile the font
$GRIT default.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

mv *.pal *.img ../nitrofiles/fnt

FIT_OK=0
if command -v python3 >/dev/null 2>&1 &&
   python3 -c 'import PIL' >/dev/null 2>&1; then
    FIT_OK=1
else
    echo "WARN: python3+PIL missing; bg/colmap may not match world size" >&2
fi

if [ "$FIT_OK" = "1" ]; then
    TMPD=$(mktemp -d)
    trap 'rm -rf "$TMPD"' EXIT
    if ! python3 fit_bg.py all --outdir "$TMPD"; then
        echo "WARN: fit_bg.py failed; using raw assets" >&2
        FIT_OK=0
    fi
fi

if [ "$FIT_OK" = "1" ]; then
    BG_SRC="$TMPD/forest.png"
    COL_SRC="$TMPD/colmap.png"
else
    BG_SRC=forest.png
    COL_SRC=colmap.png
fi

$GRIT "$BG_SRC" -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs

mv *.pal *.img *.map ../nitrofiles/bg

$GRIT "$COL_SRC" -ftB -fh! -g! -gB8 -mRt -mLf -p!

mv *.map ../nitrofiles/maps

cp *.wav ../nitrofiles/audio
