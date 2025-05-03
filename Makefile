TARGET		:= SolarGB
TITLE		:= SOLARGB01

LIBS = -limgui -lvitaGL -lvitashark -lSceShaccCgExt -ltaihen_stub -lSceShaccCg_stub \
	-lSceKernelDmacMgr_stub -lc -lSceCommonDialog_stub -lSceAudio_stub -lmathneon \
	-lSceDisplay_stub -lSceGxm_stub -lSceCtrl_stub -lSceTouch_stub -lm -lSceAppMgr_stub \
	-lSceAppUtil_stub -lScePower_stub -lSceLibKernel_stub -lSceSysmodule_stub

CFILES = bus.o \
	cart.o \
	cpu.o \
	emu.o \
	dma.o \
	gui.o \
	io.o \
	ppu.o \
	ram.o \
	timer.o

OBJS     := $(CFILES)

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -fsigned-char -Wl,-q -O3 -g -fno-optimize-sibling-calls \
	-ffast-math -mtune=cortex-a9 -mfpu=neon
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself -c -s $< eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE) -d ATTRIBUTE2=12 "$(TARGET)" param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin $(TARGET).vpk

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS) $(TARGET).elf.unstripped.elf $(TARGET).vpk eboot.bin param.sfo