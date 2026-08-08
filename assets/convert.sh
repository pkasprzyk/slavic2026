#!/bin/bash -xeu

BLOCKSDS="${BLOCKSDS:-/opt/wonderful/thirdparty/blocksds/core/}"
GRIT=$BLOCKSDS/tools/grit/grit

mkdir -p ../nitrofiles/spr ../nitrofiles/bg ../nitrofiles/fnt ../nitrofiles/maps ../nitrofiles/audio

# =========== GENERATE BG + COLMAP FROM TMX

python3 generate_from_tmx.py

# =========== SPRITES

$GRIT sprite_water_drops.png sprite_robot_all.png sprite_hare_idle.png sprite_fire_small.png indicator_arrow.png pump_body.png pump_handle.png -ftB -fh! -gTFF00FF -gt -gB8 -pS -Odefault_sprite.pal

mv *.pal *.img ../nitrofiles/spr

# =========== FONT

$GRIT default.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

mv *.pal *.img ../nitrofiles/fnt

# =========== BG

$GRIT fire_tileset.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

$GRIT UI_background.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT UI_reactor1.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT UI_reactor2.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT UI_reactor3.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT UI_reactor4.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT UI_water_level.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs

$GRIT fire_tileset.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

mv *.pal *.img ../nitrofiles/bg

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

# =========== MENU / INSTRUCTIONS BG PLACEHOLDERS

$GRIT menu_bg.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT instr_bg.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT bg_bad_ending.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT bg_mid_ending.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
$GRIT bg_good_ending.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs
mv *.pal *.img *.map ../nitrofiles/bg

# =========== COLISION MAP

$GRIT "$COL_SRC" -ftB -fh! -g! -gB8 -mRt -mLf -p!

mv *.map ../nitrofiles/maps

# =========== RAW AUDIO

cp *.wav ../nitrofiles/audio
