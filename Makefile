#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

#---------------------------------------------------------------------------------
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro)
endif

include $(DEVKITARM)/base_rules

#---------------------------------------------------------------------------------
%.gba: %.elf
	@$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)
	@gbafix $@

#---------------------------------------------------------------------------------
%.elf:
	@echo linking cartridge
	@$(LD) $(LDFLAGS) -specs=$(CURDIR)/../gba.specs $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary data
# GRAPHICS is a list of directories containing files to be processed by grit
#
# All directories are specified relative to the project directory where
# the makefile is found
#
#---------------------------------------------------------------------------------
TARGET		:= $(notdir $(CURDIR))
BUILD		:= build
SOURCES		:= source
INCLUDES	:= include
DATA		:= data

#---------------------------------------------------------------------------------
#	game code could be thumb
#---------------------------------------------------------------------------------
ARCH	:=	-marm

#		
#	fPIC => this does use a .got though			need to remove -g at some point 
CFLAGS	:= -g -Wall -O3 \
		-mcpu=arm7tdmi -mtune=arm7tdmi \
 		-fomit-frame-pointer \
		-nostdlib -nodefaultlibs \
		-D__GBA__ \
		$(ARCH)
		
ifeq ($(DEBUG), 1)
		CFLAGS += \
			-DPROFILER_ENABLE \
			-DDEBUG=1 \
			-DDEBUG_VARS=1
endif

CFLAGS	+=	$(INCLUDE)

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	:=	-g $(ARCH) -Wl,-Map,$(notdir $*.map) -T $(CURDIR)/../gba_cart.ld

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= 

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------


ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export OUTPUT	:=	$(CURDIR)/$(TARGET)
 
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

C_FILES		:=	\
				common/audio/audio.mixer.gba.c \
				common/audio/audio.sfx.c \
				common/audio/audio.stream.c \
				common/audio/audio.system.c \
				common/memory.c \
				common/memory.gba.c \
				common/string.c \
				common/stringstore.c \
				common/math/fixed16.c \
				common/math/trigonometry.c \
				common/collision/collision.c \
				common/containers/list.c \
				common/compression/lz77.gba.c \
				common/graphics/camera.c  \
				common/graphics/graphics.c  \
				common/graphics/graphics.gba.c  \
				common/graphics/graphics.clip_polygon.win32.c \
				common/graphics/overlay.c  \
				common/graphics/overlay.gba.c  \
				common/graphics/text.c \
				common/graphics/model.c  \
				common/graphics/image.c  \
				common/input/input.gba.c \
				common/states/state.c \
				common/states/debug/state_analysis.c \
				common/states/debug/state_depthmap.c \
				common/states/debug/state_image.c \
				common/states/debug/state_highlightfield.c \
				common/states/debug/state_model.c \
				common/states/debug/state_palette.c \
				common/states/debug/state_resources_inspector.c \
				common/states/debug/state_audio_sfx.c \
				common/states/debug/state_audio_stream.c \
				common/states/debug/state_stringstore.c \
				common/states/debug/state_tiledimage.c \
				common/states/debug/state_video.c \
				common/platform/gba/interrupt.c \
				common/savegame/savegame.c \
				common/savegame/savegame.gba.c \
				common/video/smacker.c \
				common/video/smacker.render_audio.c \
				common/video/smk_hufftree.c \
				common/video/smk_hufftree_v5.c \
				common/resources/resources.c \
				common/resources/resources.gba.c \
				common/utils/timer.c \
				common/utils/bitstream.c \
				common/utils/coroutine.c  \
				common/utils/random1k.c \
				common/utils/profiler.gba.c \
				common/utils/ringbuffer.c \
				games/7days/states/state_level.c \
				games/7days/states/state_dialogue.c \
				games/7days/states/state_title.c \
				games/7days/states/state_credits.c \
				games/7days/states/state_elevator.c \
				games/7days/states/state_language_select.c \
				games/7days/states/state_gameover.c \
				games/7days/states/state_inventory.c \
				games/7days/states/state_play.c \
				games/7days/states/state_pause.c \
				games/7days/states/state_savegame.c \
				games/7days/states/state_settings.c \
				games/7days/states/state_splash.c \
				games/7days/states/state_terminal.c \
				games/7days/states/state_sandbox.c \
				games/7days/widgets/widget_quickaction.c \
				games/7days/entities/model_entity.c \
				games/7days/entities/player_entity.c \
				games/7days/entities/animation.c \
				games/7days/inventory.c \
				games/7days/itemstore.c \
				games/7days/sequence.c \
				games/7days/navigation.c \
				games/7days/world.c \
				games/7days/world_sequence.c \
				main.gba.c
S_FILES		:=	\
				common/compression/lz77.decompress.gba.s \
				common/containers/sort.gba.s \
				common/platform/gba/interrupt.gba.s \
				common/audio/audio.mixer.mix_samples.gba.s \
				common/graphics/graphics.draw_spans.gba.s

ifeq ($(DEBUG), 1)
	C_FILES		+= \
				common/debug/debug.gba.c \
				common/debug/debug_helper.c 
endif

BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.res)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
export LD	:=	$(CC)

export OFILES_BIN := $(addsuffix .o,$(BINFILES))

export OFILES_SOURCES := \
				$(C_FILES:.c=.o) \
				$(S_FILES:.s=.o)
 
export OFILES := \
				$(OFILES_BIN) \
				$(OFILES_SOURCES)

export HFILES := $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),-iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(SOURCES),-iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)
 
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean
 
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || \
		mkdir -p $@ && \
		mkdir -p $@/common/audio && \
		mkdir -p $@/common/debug && \
		mkdir -p $@/common/graphics && \
		mkdir -p $@/common/math && \
		mkdir -p $@/common/collision && \
		mkdir -p $@/common/compression && \
		mkdir -p $@/common/containers && \
		mkdir -p $@/common/input && \
		mkdir -p $@/common/savegame && \
		mkdir -p $@/common/states && \
		mkdir -p $@/common/states/debug && \
		mkdir -p $@/common/platform/gba && \
		mkdir -p $@/common/resources && \
		mkdir -p $@/common/video && \
		mkdir -p $@/common/utils && \
		mkdir -p $@/games/7days && \
		mkdir -p $@/games/7days/states && \
		mkdir -p $@/games/7days/widgets && \
		mkdir -p $@/games/7days/entities 

	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).gba 


#---------------------------------------------------------------------------------
else
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

$(OUTPUT).gba	:	$(OUTPUT).elf

$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SOURCES) : $(HFILES)

#---------------------------------------------------------------------------------
# The bin2o rule should be copied and modified
# for each extension used in the data directories
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
%.res.o	%_res.h :	%.res
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------

-include $(DEPSDIR)/*.d

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
