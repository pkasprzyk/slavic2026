# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2024

BLOCKSDS	?= /opt/blocksds/core

# User config

NAME		:= sprites_animated
GAME_TITLE	:= Animated sprites
GAME_SUBTITLE	:= 2D graphics: Sprites
COMPDB = 1

# Source code paths
# -----------------

GFXDIRS		:= graphics

# Libraries
# ---------

LIBS		+= -lnflib -ldswifi9 -lnds9 -lc
LIBDIRS		+= $(BLOCKSDSEXT)/nflib \
		   $(BLOCKSDS)/libs/dswifi \
		   $(BLOCKSDS)/libs/libnds

include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile


run: $(ROM)
	@echo "  RUN $(ROM)"
	melonDS $<
