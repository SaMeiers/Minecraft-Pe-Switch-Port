.SUFFIXES:

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

VERSION_MAJOR := 0
VERSION_MINOR := 6
VERSION_MICRO := 1

APP_TITLE	:=	Minecraft PE
APP_AUTHOR	:=	SaMeiers
APP_VERSION	:=	${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}

TARGET		:=	$(subst $e ,_,$(notdir $(APP_TITLE)))
OUTDIR		:=	out
BUILD		:=	build
SOURCES		:=	handheld/src \
				handheld/src/client \
				handheld/src/locale \
				handheld/src/nbt \
				handheld/src/network \
				handheld/src/platform \
				handheld/src/raknet \
				handheld/src/server \
				handheld/src/util \
				handheld/src/world \
				handheld/src/client/gamemode \
				handheld/src/client/gui \
				handheld/src/client/model \
				handheld/src/client/multiplayer \
				handheld/src/client/particle \
				handheld/src/client/player \
				handheld/src/client/renderer \
				handheld/src/client/sound \
				handheld/src/client/gui/components \
				handheld/src/client/gui/screens \
				handheld/src/client/gui/screens/crafting \
				handheld/src/client/gui/screens/touch \
				handheld/src/client/model/geom \
				handheld/src/client/player/input \
				handheld/src/client/player/input/touchscreen \
				handheld/src/client/renderer/culling \
				handheld/src/client/renderer/entity \
				handheld/src/client/renderer/ptexture \
				handheld/src/client/renderer/tileentity \
				handheld/src/client/renderer/culling/tmp \
				handheld/src/client/sound/data \
				handheld/src/network/command \
				handheld/src/network/packet \
				handheld/src/platform/audio \
				handheld/src/platform/input \
				handheld/src/world/entity \
				handheld/src/world/food \
				handheld/src/world/inventory \
				handheld/src/world/item \
				handheld/src/world/level \
				handheld/src/world/phys \
				handheld/src/world/entity/ai \
				handheld/src/world/entity/animal \
				handheld/src/world/entity/item \
				handheld/src/world/entity/monster \
				handheld/src/world/entity/player \
				handheld/src/world/entity/projectile \
				handheld/src/world/entity/ai/control \
				handheld/src/world/entity/ai/goal \
				handheld/src/world/entity/ai/util \
				handheld/src/world/entity/ai/goal/target \
				handheld/src/world/item/crafting \
				handheld/src/world/level/biome \
				handheld/src/world/level/chunk \
				handheld/src/world/level/dimension \
				handheld/src/world/level/levelgen \
				handheld/src/world/level/material \
				handheld/src/world/level/pathfinder \
				handheld/src/world/level/storage \
				handheld/src/world/level/tile \
				handheld/src/world/level/chunk/storage \
				handheld/src/world/level/levelgen/feature \
				handheld/src/world/level/levelgen/synth \
				handheld/src/world/level/tile/entity

DATA		:=	data
INCLUDES	:=	handheld/src handheld/src/raknet handheld/src/client handheld/src/platform
EXEFS_SRC	:=	exefs_src
ROMFS		:=	romfs

ARCH	:=	-march=armv8-a -mtune=cortex-a57 -fPIE

CFLAGS	:=	-g -O2 -ffunction-sections \
			-Wall -Wno-write-strings \
			$(ARCH) $(DEFINES) \
			-DVERSION_MAJOR=${VERSION_MAJOR} \
			-DVERSION_MINOR=${VERSION_MINOR} \
			-DVERSION_MICRO=${VERSION_MICRO} \
			`$(DEVKITPRO)/portlibs/switch/bin/aarch64-none-elf-pkg-config --cflags freetype2` \
			`$(DEVKITPRO)/portlibs/switch/bin/aarch64-none-elf-pkg-config --cflags sdl2`

CFLAGS	+=	$(INCLUDE) -D__SWITCH__ -D_GNU_SOURCE=1

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++14

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-no-as-needed,-Map,$(notdir $*.map)

LIBS	:=	-lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2_mixer \
			-lpng -ljpeg -lwebp \
			-lvorbisidec -logg -lmpg123 -lmodplug -lopusfile -lopus -lFLAC \
			`$(DEVKITPRO)/portlibs/switch/bin/aarch64-none-elf-pkg-config --libs sdl2` \
			`$(DEVKITPRO)/portlibs/switch/bin/aarch64-none-elf-pkg-config --libs freetype2` \
			-lharfbuzz -lfreetype -lbz2 -lpng -lz \
			-lGLESv1_CM -lEGL -lglapi -ldrm_nouveau \
			-lnx -lm

LIBDIRS	:= $(PORTLIBS) $(LIBNX)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(OUTDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

CPPFILES := $(filter-out main_android.cpp AppPlatform_android.cpp,$(CPPFILES))
CPPFILES := $(filter-out main_android_java.cpp,$(CPPFILES))
CPPFILES := $(filter-out main_win32.cpp AppPlatform_win32.cpp,$(CPPFILES))
CPPFILES := $(filter-out main_rpi.cpp,$(CPPFILES))
CPPFILES := $(filter-out main_dedicated.cpp,$(CPPFILES))
CPPFILES := $(filter-out main_switch.cpp AppPlatform_switch.cpp,$(CPPFILES))
CPPFILES := $(filter-out main_switch_minimal.cpp,$(CPPFILES))
CPPFILES := $(filter-out SoundSystemSL.cpp SoundSystemAL.cpp,$(CPPFILES))
CPPFILES := $(filter-out time.cpp,$(CPPFILES))
# Exclude Switch mock implementations now that we link libGLESv1_CM
CPPFILES := $(filter-out switch_missing.cpp,$(CPPFILES))

ifeq ($(strip $(CPPFILES)),)

	export LD	:=	$(CC)

else

	export LD	:=	$(CXX)

endif

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export BUILD_EXEFS_SRC := $(TOPDIR)/$(EXEFS_SRC)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.jpg)
	ifneq (,$(findstring $(TARGET).jpg,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).jpg
	else
		ifneq (,$(findstring icon.jpg,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.jpg
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
	export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
	export NROFLAGS += --nacp=$(CURDIR)/$(OUTDIR)/$(TARGET).nacp
endif

ifneq ($(APP_TITLEID),)
	export NACPFLAGS += --titleid=$(APP_TITLEID)
endif

ifneq ($(ROMFS),)
	export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@ $(BUILD) $(OUTDIR)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTDIR)


else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

all	:	$(OUTPUT).pfs0 $(OUTPUT).nro

$(OUTPUT).pfs0	:	$(OUTPUT).nso

$(OUTPUT).nso	:	$(OUTPUT).elf

ifeq ($(strip $(NO_NACP)),)
$(OUTPUT).nro	:	$(OUTPUT).elf $(OUTPUT).nacp
else
$(OUTPUT).nro	:	$(OUTPUT).elf
endif

$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES_BIN)

%.bin.o	%_bin.h :	%.bin

	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif

