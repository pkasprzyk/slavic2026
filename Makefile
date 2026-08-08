# SPDX-License-Identifier: CC0-1.0

export BLOCKSDS			?= /opt/wonderful/thirdparty/blocksds/core
export BLOCKSDSEXT		?= /opt/wonderful/thirdparty/blocksds/external
export WONDERFUL_TOOLCHAIN	?= /opt/wonderful
ARM_NONE_EABI_PATH	?= $(WONDERFUL_TOOLCHAIN)/toolchain/gcc-arm-none-eabi/bin/

# User config
# -----------

NAME		:= slavic2026
GAME_TITLE	:= Chilling Mech
GAME_SUBTITLE	:= Slavic Game Jam 2026
GAME_AUTHOR	:= fisz
GAME_ICON	:= $(BLOCKSDS)/sys/icon.bmp

COMPDB = 1

SOURCEDIRS	?= source
NITROFSDIR	?= nitrofiles
AUDIODIRS	?= audio

# Libraries
# ---------

LIBS		+= -lnflib -lmm9 -ldswifi9 -lnds9 -lc
LIBDIRS		+= $(BLOCKSDSEXT)/nflib \
		   $(BLOCKSDS)/libs/dswifi \
		   $(BLOCKSDS)/libs/libnds \
		   $(BLOCKSDS)/libs/maxmod

# Build artifacts
# ---------------

BUILDDIR	:= build/$(NAME)
ELF		:= build/$(NAME).elf
DUMP		:= build/$(NAME).dump
MAP		:= build/$(NAME).map
ROM		:= $(NAME).nds

SOUNDBANKINFODIR	:= $(BUILDDIR)/maxmod
SOUNDBANKDIR		:= $(BUILDDIR)/maxmod_nitrofs

# Tools
# -----

PREFIX		:= $(ARM_NONE_EABI_PATH)arm-none-eabi-
CC		:= $(PREFIX)gcc
LD		:= $(PREFIX)gcc
OBJDUMP		:= $(PREFIX)objdump
MKDIR		:= mkdir
RM		:= rm -rf

ifeq ($(VERBOSE),1)
V		:=
else
V		:= @
endif

# Source files
# ------------

SOURCES_S	:= $(shell find -L $(SOURCEDIRS) -name "*.s")
SOURCES_C	:= $(shell find -L $(SOURCEDIRS) -name "*.c")
SOURCES_AUDIO	:= $(shell find -L $(AUDIODIRS) -regex '.*\.\(it\|mod\|s3m\|wav\|xm\)')

# Compiler and linker flags
# -------------------------

ARCH		:= -mthumb -mcpu=arm946e-s+nofp
SPECS		:= $(BLOCKSDS)/sys/crts/ds_arm9.specs
WARNFLAGS	:= -Wall

INCLUDEFLAGS	:= $(foreach path,$(LIBDIRS),-I$(path)/include) \
		   -I$(SOUNDBANKINFODIR)
LIBDIRSFLAGS	:= $(foreach path,$(LIBDIRS),-L$(path)/lib)

ASFLAGS		+= -x assembler-with-cpp $(INCLUDEFLAGS) $(ARCH) \
		   -ffunction-sections -fdata-sections \
		   -specs=$(SPECS)

CFLAGS		+= -std=gnu17 $(WARNFLAGS) $(INCLUDEFLAGS) $(ARCH) \
		   -O2 -ffunction-sections -fdata-sections \
		   -specs=$(SPECS)

LDFLAGS		:= $(ARCH) $(LIBDIRSFLAGS) -Wl,-Map,$(MAP) \
		   -Wl,--start-group $(LIBS) -Wl,--end-group -specs=$(SPECS)

# Intermediate build files
# ------------------------

ifneq ($(SOURCES_AUDIO),)
    HEADERS_ASSETS	+= $(SOUNDBANKINFODIR)/soundbank.h
endif

OBJS		:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_S))) \
		   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_C)))

DEPS		:= $(OBJS:.o=.d)

# Targets
# -------

.PHONY: all clean dump run

all: $(ROM)

ifeq ($(COMPDB),1)
all: $(BUILDDIR)/compile_commands.json
endif

NDSTOOL_ARGS	:= -d $(NITROFSDIR)
ifneq ($(SOURCES_AUDIO),)
    NDSTOOL_ARGS	+= -d $(SOUNDBANKDIR)
endif

# Asset conversion: regenerate nitrofiles/ from assets/ via GRIT
NITROFS_ASSETS	:= $(shell find -L assets -name "*.png") assets/convert.sh source/level.h

$(NITROFSDIR)/.converted: $(NITROFS_ASSETS)
	@echo "  CONVERT assets -> nitrofiles"
	$(V)cd assets && bash convert.sh
	$(V)touch $@

$(ROM): $(NITROFSDIR)/.converted

ifeq ($(strip $(GAME_SUBTITLE)),)
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_AUTHOR)
else
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_SUBTITLE);$(GAME_AUTHOR)
endif

$(ROM): $(ELF)
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 $(BLOCKSDS)/sys/default_arm7/arm7.elf -9 $(ELF) \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_ARGS)

$(ELF): $(OBJS)
	@echo "  LD      $@"
	$(V)$(LD) -o $@ $(OBJS) $(LDFLAGS)

$(DUMP): $(ELF)
	@echo "  OBJDUMP   $@"
	$(V)$(OBJDUMP) -h -C -S $< > $@

dump: $(DUMP)

clean:
	@echo "  CLEAN"
	$(V)$(RM) $(ROM) $(DUMP) build compile_commands.json
	$(V)$(RM) -fr $(NITROFSDIR)

ifeq ($(COMPDB),1)
$(BUILDDIR)/compile_commands.json: $(OBJS)
	@echo "  MERGE   compile_commands.json"
	$(V)$(WONDERFUL_TOOLCHAIN)/bin/wf-compile-commands-merge $@ $(patsubst %.o,%.cc.json,$^)
	$(V)cp $@ $(CURDIR)/compile_commands.json
endif

# Rules
# -----

$(BUILDDIR)/%.s.o : %.s
	@echo "  AS      $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(ASFLAGS) -MMD -MP -MJ $(patsubst %.o,%.cc.json,$@) -c -o $@ $<

$(BUILDDIR)/%.c.o : %.c
	@echo "  CC      $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -MJ $(patsubst %.o,%.cc.json,$@) -c -o $@ $<

ifneq ($(SOURCES_AUDIO),)

$(SOUNDBANKINFODIR)/soundbank.h: $(SOURCES_AUDIO)
	@echo "  MMUTIL  $^"
	@$(MKDIR) -p $(SOUNDBANKDIR)
	@$(MKDIR) -p $(SOUNDBANKINFODIR)
	@$(BLOCKSDS)/tools/mmutil/mmutil $^ -d \
		-o$(SOUNDBANKDIR)/soundbank.bin -h$(SOUNDBANKINFODIR)/soundbank.h

endif

$(SOURCES_S) $(SOURCES_C): $(HEADERS_ASSETS)

-include $(DEPS)

run: all
	@echo "  RUN $(ROM)"
	melonDS $(ROM)
