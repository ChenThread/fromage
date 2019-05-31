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
	-Isrc -I$(CANDYK)/include -Wall -Wextra \
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

INCLUDES = src/psx.h src/common.h
# $(OBJDIR)/head.o \
#	\

OBJS =	$(OBJDIR)/gui.o \
	$(OBJDIR)/gpu.o \
	$(OBJDIR)/world.o \
	\
	$(OBJDIR)/atlas.o \
	\
	$(OBJDIR)/main.o


all: $(EXE_NAME).exe $(ISO_NAME).cue

clean:
	$(RM_F) $(OBJS) $(OBJDIR)/$(EXE_NAME).elf $(ISO_NAME).bin $(ISO_NAME).cue $(OBJDIR)/atlas.s

$(ISO_NAME).cue: $(ISO_NAME)
	$(CANDYK)/bin/pscd-new manifest.txt

$(ISO_NAME): $(EXE_NAME).exe
	$(MKISOFS) -o $(ISO_NAME) system.cnf $(EXE_NAME).exe

$(EXE_NAME).exe: $(OBJDIR)/$(EXE_NAME).elf
	$(CANDYK)/bin/elf2psx -p $(OBJDIR)/$(EXE_NAME).elf $(EXE_NAME).exe

$(OBJDIR)/$(EXE_NAME).elf: $(OBJS)
	$(CROSS_CC) -o $(OBJDIR)/$(EXE_NAME).elf $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/atlas.o: $(DATDIR)/atlas.raw tools/bin2s.py $(INCLUDES)
	$(PYTHON3) tools/bin2s.py $(DATDIR)/atlas.raw > $(OBJDIR)/atlas.s
	$(CROSS_AS) -o $@ $(ASFLAGS) $(OBJDIR)/atlas.s

