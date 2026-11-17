CROSS_CC_PREFIX = /djgpp/bin/i586-pc-msdosdjgpp-
CC = $(CROSS_CC_PREFIX)gcc
MKDIR = mkdir -p
OBJS :=

# Common
OBJS += \
	src/town.o \
	src/mainout.o \
	src/mainoutb.o \
	src/dungeon.o \
	src/dungeonb.o \
	src/intro0.o \
	src/intro.o \
	src/intro2.o \
	src/introb.o \
	src/npc.o \
	src/combat.o \
	src/blckthrn.o \
	src/lookobj.o \
	src/dnglook.o \
	src/outsubs.o \
	src/outsubsb.o \
	src/shoppes.o \
	src/endgame.o \
	src/sjog.o \
	src/cmds.o \
	src/cmds2.o \
	src/cmdsb.o \
	src/cast.o \
	src/castb.o \
	src/talk.o \
	src/cast2.o \
	src/zstats.o \
	src/comsubs.o \
	src/shoppes2.o \
	src/shoppe2b.o \
	src/shoppes3.o \
	src/font.o \
	src/vars.o \
	src/ultima.o \
	src/0000.o \
	src/1000.o \
	src/2000.o \
	src/r2320.o \
	src/r251e.o \
	src/3000.o \
	src/4000.o \
	src/5000.o \
	src/6000.o \
	src/7000.o \
	src/r0d72.o \
	src/r1850.o \
	src/r1b38.o \
	src/r1dda.o \
	src/r1fa0.o \
	src/r216c.o \
	src/savegame.o \
	src/grap_drv.o \
	src/audio/audio.o \
	src/audio/aud_adl.o \
	src/audio/aud_cdda.o \
	src/audio/aud_mus.o \
	src/audio/aud_spk.o \
	src/audio/aud_sfx.o \
	src/backend/backend.o \
	src/common/file.o \
	src/common/util.o \
    src/common/settings.o \
	src/event/evt_dos.o \
	src/graphics/animate.o \
	src/graphics/image.o \
	src/graphics/grap_buf.o \
	src/graphics/origin.o \
	src/graphics/grap_vga.o \
	src/graphics/grap.o \
	src/graphics/reveal.o \
	src/graphics/wd.o \
	src/common/debug.o \
	src/common/lzw.o \
	src/common/memory.o \
	src/key/key_dos.o \
	src/time/time_dos.o

SRCS = $(OBJS:.o=.c)
TARGET = ultima5.exe

DEPDIR := .deps
MODULE_DIRS := ./ ./src/ ./src/audio/ ./src/backend/ ./src/common/ ./src/event/ ./src/graphics/ ./src/key/ ./src/time/
DEPDIRS := $(addsuffix $(DEPDIR),$(MODULE_DIRS))

INCLUDE_PATH := ./src/
LIB_PATH := 

C_FLAGS :=
LD_FLAGS :=

C_FLAGS += -I$(INCLUDE_PATH)
#LD_FLAGS += -L$(LIB_PATH)

S_FLAGS +=
C_FLAGS += -DDOS
C_FLAGS += -O2 -s
C_FLAGS += -fcommon # vars.c and the modules both define the data (common symbols)
LD_FLAGS += -lemu

C_UPDATE_DEP_FLAG = -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d"

.SUFFIXES: .c .o

$(shell $(MKDIR) $(DEPDIRS))

%.o: %.c
	$(CC) $(C_UPDATE_DEP_FLAG) $(C_FLAGS) -c $(<) -o $*.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LD_FLAGS)

-include $(wildcard $(addsuffix /*.d,$(DEPDIRS)))

clean:
	rm -f $(OBJS) $(TARGET)
