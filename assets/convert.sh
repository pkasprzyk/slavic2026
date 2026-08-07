#!/bin/bash -xeu

BLOCKSDS="${BLOCKSDS:-/opt/wonderful/thirdparty/blocksds/core/}"
GRIT=$BLOCKSDS/tools/grit/grit

mkdir -p ../nitrofiles/spr ../nitrofiles/bg ../nitrofiles/fnt ../nitrofiles/maps ../nitrofiles/audio

$GRIT mech.png -ftB -fh! -gTFF00FF -gt -gB8 -m!
$GRIT sprite_robot_idle.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

mv *.pal *.img ../nitrofiles/spr

# compile the font
$GRIT default.png -ftB -fh! -gTFF00FF -gt -gB8 -m!

mv *.pal *.img ../nitrofiles/fnt

$GRIT forest.png -ftB -fh! -gTFF00FF -gt -gB8 -mR8 -mLs

mv *.pal *.img *.map ../nitrofiles/bg

$GRIT colmap.png -ftB -fh! -g! -gB8 -mRt -mLf -p!

mv *.map ../nitrofiles/maps

cp *.wav ../nitrofiles/audio
