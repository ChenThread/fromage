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

LIBS = -lm -lsawpads -lc -lgcc -lchenboot

# stuff omitted:
# O2:
# O3:
# -funswitch-loops - slows things down
# -fipa-cp-clone - also slows things down
#

EXE_NAME=boot
ISO_NAME=fromage

DATDIR = dat
OBJDIR = obj
SRCDIR = src

INCLUDES = src/block_info.h src/common.h src/config.h src/psx.h

OBJS =	$(OBJDIR)/cdrom.o \
	$(OBJDIR)/gui.o \
	$(OBJDIR)/gpu.o \
	$(OBJDIR)/gpu_dma.o \
	$(OBJDIR)/save.o \
	$(OBJDIR)/world.o \
	$(OBJDIR)/worldgen.o \
	\
	$(OBJDIR)/atlas.o \
	$(OBJDIR)/font.o \
	\
	$(OBJDIR)/lz4.o \
	\
	$(OBJDIR)/main.o


all: $(EXE_NAME).exe $(ISO_NAME).cue

clean:
	$(RM_F) $(OBJS) $(OBJDIR)/$(EXE_NAME).elf $(ISO_NAME).bin $(ISO_NAME).cue $(OBJDIR)/atlas.s $(OBJDIR)/font.s

$(ISO_NAME).cue: $(ISO_NAME) manifest.txt
	$(CANDYK)/bin/pscd-new manifest.txt

$(ISO_NAME): $(EXE_NAME).exe
	$(MKISOFS) -o $(ISO_NAME) system.cnf $(EXE_NAME).exe

$(EXE_NAME).exe: $(OBJDIR)/$(EXE_NAME).elf
	$(CANDYK)/bin/elf2psx -p $(OBJDIR)/$(EXE_NAME).elf $(EXE_NAME).exe

$(OBJDIR)/$(EXE_NAME).elf: $(OBJS)
	$(CROSS_CC) -o $(OBJDIR)/$(EXE_NAME).elf $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/lz4.o: contrib/lz4/lz4.c contrib/lz4/lz4.h
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/atlas.o: $(DATDIR)/atlas.raw tools/bin2s.py $(INCLUDES)
	$(PYTHON3) tools/bin2s.py $(DATDIR)/atlas.raw > $(OBJDIR)/atlas.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/atlas.s

$(OBJDIR)/font.o: $(DATDIR)/font.raw tools/bin2s.py $(INCLUDES)
	$(PYTHON3) tools/bin2s.py $(DATDIR)/font.raw > $(OBJDIR)/font.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/font.s

