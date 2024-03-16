CANDYK=../candyk-psx
CROSSPREFIX=/opt/wonderful/toolchain/gcc-mipsel-elf/bin/mipsel-elf-

CROSS_CC=$(CROSSPREFIX)gcc
CROSS_AS=$(CROSSPREFIX)as
CROSS_OBJCOPY=$(CROSSPREFIX)objcopy

RM_F=rm -f
MKISOFS=mkisofs
PYTHON3=python3

SPUENC=$(CANDYK)/bin/psxavenc

ASFLAGS = -g -msoft-float

CFLAGS = -g -c -flto -pipe \
	-fomit-frame-pointer \
	-fno-stack-protector \
	-mno-check-zero-division \
	-msoft-float -nostdlib -mips1 -march=3000 -mtune=3000 \
	-Isrc -Icontrib/lz4 -I$(CANDYK)/include -Wall -Wextra \
	-Wno-shift-negative-value \
	-Wno-unused-variable -Wno-unused-function -Wno-pointer-sign \

CFLAGS_FAST = -O3
CFLAGS_SMALL = -Os
ELF2PSXFLAGS =

LDFLAGS	:= -Wl,--defsym=vfprintf=__i_vfprintf -Wl,--defsym=vfscanf=__i_vfscanf \
	-g -O3 -flto -Wl,-Tlink.ld -pipe \
	-mtune=3000 -march=3000 \
	-fomit-frame-pointer \
	-fno-stack-protector \
	-mno-check-zero-division \
	\
	-msoft-float \
	-L$(CANDYK)/lib

LIBS = -lm -lorelei -lsawpads -lseedy -lc -lgcc -lchenboot

# stuff omitted:
# O2:
# O3:
# -funswitch-loops - slows things down
# -fipa-cp-clone - also slows things down
#

EXE_NAME=boot
ISO_NAME?=fromage

OBJDIR = obj
RESDIR ?= res
SRCDIR = src
TOOLSDIR = tools
TOOLSOBJDIR = $(OBJDIR)/tools

INCLUDES = src/block_info.h src/common.h src/config.h

MUSIC = \
	$(OBJDIR)/calm1.mus \
	$(OBJDIR)/calm2.mus \
	$(OBJDIR)/calm3.mus

SOUNDS = \
	$(OBJDIR)/grass1.snd \
	$(OBJDIR)/grass2.snd \
	$(OBJDIR)/grass3.snd \
	$(OBJDIR)/grass4.snd \
	$(OBJDIR)/gravel1.snd \
	$(OBJDIR)/gravel2.snd \
	$(OBJDIR)/gravel3.snd \
	$(OBJDIR)/gravel4.snd \
	$(OBJDIR)/stone1.snd \
	$(OBJDIR)/stone2.snd \
	$(OBJDIR)/stone3.snd \
	$(OBJDIR)/stone4.snd \
	$(OBJDIR)/wood1.snd \
	$(OBJDIR)/wood2.snd \
	$(OBJDIR)/wood3.snd \
	$(OBJDIR)/wood4.snd

OBJS_FAST = \
	$(OBJDIR)/cdrom.o \
	$(OBJDIR)/gpu.o \
	$(OBJDIR)/gpu_dma.o \
	$(OBJDIR)/joy.o \
	$(OBJDIR)/sound.o \
	$(OBJDIR)/util.o \
	$(OBJDIR)/world.o \
	$(OBJDIR)/worldgen.o \
	\
	$(OBJDIR)/main.o

OBJS_SMALL = \
	$(OBJDIR)/options.o \
	$(OBJDIR)/gui.o \
	$(OBJDIR)/save.o \
	\
	$(OBJDIR)/font1x.o \
	$(OBJDIR)/font2x.o \
	$(OBJDIR)/icon.o \
	$(OBJDIR)/license_text.o \
	\
	$(OBJDIR)/lz4.o

# uncomment for standalone boot.exe build
ifeq ($(TYPE),exe)
CFLAGS += -DSTANDALONE_EXE
endif

# uncomment for PAL build
ifeq ($(REGION),europe)
CFLAGS += -DREGION_EUROPE
ELF2PSXFLAGS += -p
endif

# uncomment for NTSC build
ifeq ($(REGION),usa)
CFLAGS += -DREGION_USA
ELF2PSXFLAGS += -n
endif

ifeq ($(VIDEO_WIDTH),640)
CFLAGS += -DVID_USE_640
endif

OBJS =  $(OBJS_FAST) $(OBJS_SMALL)

ifeq ($(TYPE),exe)
all: $(EXE_NAME).exe
else
all: $(EXE_NAME).exe $(ISO_NAME).cue
endif

clean:
	$(RM_F) $(OBJS) $(OBJDIR)/$(EXE_NAME).elf $(ISO_NAME).bin $(ISO_NAME).cue
	$(RM_F) $(OBJDIR)/font.s $(OBJDIR)/icon.s
	$(RM_F) $(OBJDIR)/atlas.raw $(OBJDIR)/font.raw $(OBJDIR)/icon.raw
	$(RM_F) atlas.lz4 sounds.lz4 $(OBJDIR)/atlas.lz4.h
	$(RM_F) $(OBJDIR)/atlas.lz4.o
	$(RM_F) $(OBJDIR)/soundbank.raw
	$(RM_F) $(SOUNDS)
	$(RM_F) $(MUSIC) music.hdr music.xa
	$(RM_F) $(OBJDIR)/license_text.s $(OBJDIR)/license_text.txt

$(ISO_NAME).cue: $(ISO_NAME) music.hdr music.xa atlas.lz4 sounds.lz4 manifest.txt
	$(CANDYK)/bin/pscd-new manifest.txt

$(ISO_NAME): $(EXE_NAME).exe
	$(MKISOFS) -o $(ISO_NAME) system.cnf $(EXE_NAME).exe

$(EXE_NAME).exe: $(OBJDIR)/$(EXE_NAME).elf
	$(CANDYK)/bin/elf2psx $(ELF2PSXFLAGS) $(OBJDIR)/$(EXE_NAME).elf $(EXE_NAME).exe

$(OBJDIR)/$(EXE_NAME).elf: $(OBJS) link.ld
	$(CROSS_CC) -o $(OBJDIR)/$(EXE_NAME).elf $(LDFLAGS) $(OBJS) $(LIBS)

music.hdr: $(MUSIC) $(RESDIR)/music_volume.txt
	$(PYTHON3) $(TOOLSDIR)/mkmusichdr.py music.hdr $(RESDIR)/music_volume.txt $(MUSIC)

music.xa: $(MUSIC) $(RESDIR)/music.txt
	$(CANDYK)/bin/xainterleave $(RESDIR)/music.txt music.xa

$(OBJDIR)/soundbank.raw: $(SOUNDS)
	$(PYTHON3) $(TOOLSDIR)/mksoundbank.py $(OBJDIR)/soundbank.raw $(SOUNDS)

$(OBJDIR)/%.snd: $(RESDIR)/%.ogg
	$(SPUENC) -f 22050 -t spu -c 1 -b 4 $< $@

$(OBJDIR)/%.snd: $(RESDIR)/%.wav
	$(SPUENC) -f 22050 -t spu -c 1 -b 4 $< $@

$(OBJDIR)/calm1.mus: $(RESDIR)/calm1.ogg
	$(SPUENC) -f 37800 -t xacd -c 2 -b 4 -F 1 -C 0 $< $@

$(OBJDIR)/calm2.mus: $(RESDIR)/calm2.ogg
	$(SPUENC) -f 37800 -t xacd -c 2 -b 4 -F 1 -C 1 $< $@

$(OBJDIR)/calm3.mus: $(RESDIR)/calm3.ogg
	$(SPUENC) -f 37800 -t xacd -c 2 -b 4 -F 1 -C 2 $< $@

$(OBJS_FAST): CFLAGS := $(CFLAGS) $(CFLAGS_FAST)
$(OBJS_SMALL): CFLAGS := $(CFLAGS) $(CFLAGS_SMALL)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES) atlas.lz4
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(CROSS_CC) -g -c -o $@ $(CFLAGS) $<

sounds.lz4: $(OBJDIR)/soundbank.raw $(OBJDIR)/lz4pack
	$(OBJDIR)/lz4pack -o 32 -p $(OBJDIR)/soundbank.raw sounds.lz4

$(OBJDIR)/lz4.o: contrib/lz4/lz4.c contrib/lz4/lz4.h
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/lz4pack: contrib/lz4/lz4.c contrib/lz4/lz4.h tools/lz4pack.c
	$(CC) -O2 -Icontrib/lz4 -o $@ contrib/lz4/lz4.c tools/lz4pack.c

atlas.lz4: $(OBJDIR)/atlas.raw $(OBJDIR)/lz4pack $(TOOLSDIR)/bin2h.py
	$(OBJDIR)/lz4pack $(OBJDIR)/atlas.raw atlas.lz4
	$(PYTHON3) $(TOOLSDIR)/bin2h.py atlas.lz4 > $(OBJDIR)/atlas.lz4.h

$(OBJDIR)/atlas.raw: $(RESDIR)/atlas.png $(TOOLSDIR)/mkatlas.py
	$(PYTHON3) $(TOOLSDIR)/mkatlas.py $(RESDIR)/atlas.png $(OBJDIR)/atlas.raw

$(OBJDIR)/font1x.o: $(OBJDIR)/font1x.raw $(TOOLSDIR)/bin2s.py $(INCLUDES)
	$(PYTHON3) $(TOOLSDIR)/bin2s.py $(OBJDIR)/font1x.raw > $(OBJDIR)/font1x.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/font1x.s

$(OBJDIR)/font2x.o: $(OBJDIR)/font2x.raw $(TOOLSDIR)/bin2s.py $(INCLUDES)
	$(PYTHON3) $(TOOLSDIR)/bin2s.py $(OBJDIR)/font2x.raw > $(OBJDIR)/font2x.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/font2x.s

$(OBJDIR)/font1x.raw: $(RESDIR)/font.png
	$(PYTHON3) $(TOOLSDIR)/mkfont.py $(RESDIR)/font.png $(OBJDIR)/font1x.raw 1x

$(OBJDIR)/font2x.raw: $(RESDIR)/font.png
	$(PYTHON3) $(TOOLSDIR)/mkfont.py $(RESDIR)/font.png $(OBJDIR)/font2x.raw 2x

$(OBJDIR)/icon.raw: $(RESDIR)/icon.png
	$(PYTHON3) $(TOOLSDIR)/mkicon.py $(RESDIR)/icon.png $(OBJDIR)/icon.raw

$(OBJDIR)/icon.o: $(OBJDIR)/icon.raw $(TOOLSDIR)/bin2s.py $(INCLUDES)
	$(PYTHON3) $(TOOLSDIR)/bin2s.py $(OBJDIR)/icon.raw > $(OBJDIR)/icon.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/icon.s

$(OBJDIR)/license_text.txt: LICENSE $(RESDIR)/LICENSE contrib/lz4/LICENSE
	cat LICENSE > $(OBJDIR)/license_text.txt
	echo "" >> $(OBJDIR)/license_text.txt
	echo "--------" >> $(OBJDIR)/license_text.txt
	echo "" >> $(OBJDIR)/license_text.txt
	cat $(RESDIR)/LICENSE >> $(OBJDIR)/license_text.txt
	echo "" >> $(OBJDIR)/license_text.txt
	echo "--------" >> $(OBJDIR)/license_text.txt
	echo "" >> $(OBJDIR)/license_text.txt
	cat contrib/lz4/LICENSE >> $(OBJDIR)/license_text.txt

$(OBJDIR)/license_text.o: $(OBJDIR)/license_text.txt $(TOOLSDIR)/bin2s.py $(INCLUDES)
	$(PYTHON3) $(TOOLSDIR)/bin2s.py $(OBJDIR)/license_text.txt > $(OBJDIR)/license_text.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/license_text.s
