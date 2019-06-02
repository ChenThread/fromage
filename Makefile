CROSSPREFIX=mipsel-none-elf-

CROSS_CC=$(CROSSPREFIX)gcc
CROSS_AS=$(CROSSPREFIX)as
CROSS_OBJCOPY=$(CROSSPREFIX)objcopy

RM_F=rm -f
MKISOFS=mkisofs
PYTHON3=python3

ASFLAGS = -g -msoft-float

CFLAGS = -g -c -O3 -flto -pipe \
	-fomit-frame-pointer \
	-funroll-loops \
	-fno-stack-protector \
	-mno-check-zero-division \
	-msoft-float -nostdlib -mips1 -march=3000 -mtune=3000 \
	-Isrc -Icontrib/lz4 -I$(CANDYK)/include -Wall -Wextra \
	-Wno-shift-negative-value \
	-Wno-unused-variable -Wno-unused-function -Wno-pointer-sign \

LDFLAGS = -g -O3 -flto -Wl,-Ttext-segment=0x80010000 -pipe \
	-mtune=3000 -march=3000 \
	-fomit-frame-pointer \
	-fno-stack-protector \
	-funroll-loops \
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
ISO_NAME=fromage

OBJDIR = obj
RESDIR ?= res
SRCDIR = src

INCLUDES = src/block_info.h src/common.h src/config.h src/psx.h $(OBJDIR)/soundbank.h

SOUNDS = \
	$(OBJDIR)/grass1.spu \
	$(OBJDIR)/grass2.spu \
	$(OBJDIR)/grass3.spu \
	$(OBJDIR)/grass4.spu \
	$(OBJDIR)/gravel1.spu \
	$(OBJDIR)/gravel2.spu \
	$(OBJDIR)/gravel3.spu \
	$(OBJDIR)/gravel4.spu \
	$(OBJDIR)/stone1.spu \
	$(OBJDIR)/stone2.spu \
	$(OBJDIR)/stone3.spu \
	$(OBJDIR)/stone4.spu \
	$(OBJDIR)/wood1.spu \
	$(OBJDIR)/wood2.spu \
	$(OBJDIR)/wood3.spu \
	$(OBJDIR)/wood4.spu

OBJS =	$(OBJDIR)/cdrom.o \
	$(OBJDIR)/gui.o \
	$(OBJDIR)/gpu.o \
	$(OBJDIR)/gpu_dma.o \
	$(OBJDIR)/save.o \
	$(OBJDIR)/sound.o \
	$(OBJDIR)/world.o \
	$(OBJDIR)/worldgen.o \
	\
	$(OBJDIR)/atlas.o \
	$(OBJDIR)/font.o \
	$(OBJDIR)/icon.o \
	$(OBJDIR)/license_text.o \
	\
	$(OBJDIR)/lz4.o \
	\
	$(OBJDIR)/main.o


all: $(EXE_NAME).exe $(ISO_NAME).cue

clean:
	$(RM_F) $(OBJS) $(OBJDIR)/$(EXE_NAME).elf $(ISO_NAME).bin $(ISO_NAME).cue
	$(RM_F) $(OBJDIR)/atlas.s $(OBJDIR)/font.s $(OBJDIR)/icon.s
	$(RM_F) $(OBJDIR)/atlas.raw $(OBJDIR)/font.raw $(OBJDIR)/icon.raw
	$(RM_F) $(OBJDIR)/soundbank.h
	$(RM_F) $(OBJDIR)/soundbank.raw
	$(RM_F) $(SOUNDS)
	$(RM_F) $(OBJDIR)/license_text.s $(OBJDIR)/license_text.txt

$(ISO_NAME).cue: $(ISO_NAME) manifest.txt
	$(CANDYK)/bin/pscd-new manifest.txt

$(ISO_NAME): $(EXE_NAME).exe
	$(MKISOFS) -o $(ISO_NAME) system.cnf $(EXE_NAME).exe

$(EXE_NAME).exe: $(OBJDIR)/$(EXE_NAME).elf
	$(CANDYK)/bin/elf2psx -p $(OBJDIR)/$(EXE_NAME).elf $(EXE_NAME).exe

$(OBJDIR)/$(EXE_NAME).elf: $(OBJS)
	$(CROSS_CC) -o $(OBJDIR)/$(EXE_NAME).elf $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJDIR)/soundbank.raw: $(SOUNDS)
	$(PYTHON3) tools/mksoundbank.py $(OBJDIR)/soundbank.raw $(SOUNDS)

$(OBJDIR)/%.spu: $(RESDIR)/%.ogg
	spuenc -f 14700 -t spu -c 1 $< $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/soundbank.h: $(OBJDIR)/soundbank.raw
	$(PYTHON3) tools/bin2h.py $(OBJDIR)/soundbank.raw > $(OBJDIR)/soundbank.h

$(OBJDIR)/lz4.o: contrib/lz4/lz4.c contrib/lz4/lz4.h
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/atlas.o: $(OBJDIR)/atlas.raw tools/bin2s.py $(INCLUDES)
	$(PYTHON3) tools/bin2s.py $(OBJDIR)/atlas.raw > $(OBJDIR)/atlas.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/atlas.s

$(OBJDIR)/font.o: $(OBJDIR)/font.raw tools/bin2s.py $(INCLUDES)
	$(PYTHON3) tools/bin2s.py $(OBJDIR)/font.raw > $(OBJDIR)/font.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/font.s

$(OBJDIR)/soundbank.o: $(OBJDIR)/soundbank.raw tools/bin2s.py $(INCLUDES)
	$(PYTHON3) tools/bin2s.py $(OBJDIR)/soundbank.raw > $(OBJDIR)/soundbank.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/soundbank.s

$(OBJDIR)/atlas.raw: $(RESDIR)/atlas.png $(RESDIR)/water.png $(RESDIR)/lava.png
	$(PYTHON3) tools/mkatlas.py $(RESDIR)/atlas.png $(OBJDIR)/atlas.raw $(RESDIR)/water.png $(RESDIR)/lava.png

$(OBJDIR)/font.raw: $(RESDIR)/font.png
	$(PYTHON3) tools/mkfont.py $(RESDIR)/font.png $(OBJDIR)/font.raw

$(OBJDIR)/icon.raw: $(RESDIR)/icon.png
	$(PYTHON3) tools/mkicon.py $(RESDIR)/icon.png $(OBJDIR)/icon.raw

$(OBJDIR)/icon.o: $(OBJDIR)/icon.raw tools/bin2s.py $(INCLUDES)
	$(PYTHON3) tools/bin2s.py $(OBJDIR)/icon.raw > $(OBJDIR)/icon.s
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

$(OBJDIR)/license_text.o: $(OBJDIR)/license_text.txt tools/bin2s.py $(INCLUDES)
	$(PYTHON3) tools/bin2s.py $(OBJDIR)/license_text.txt > $(OBJDIR)/license_text.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/license_text.s
