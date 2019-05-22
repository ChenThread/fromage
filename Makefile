CROSSPREFIX=mipsel-none-elf-

CROSS_CC=$(CROSSPREFIX)gcc
CROSS_AS=$(CROSSPREFIX)as
CROSS_OBJCOPY=$(CROSSPREFIX)objcopy

RM_F=rm -f
MKISOFS=mkisofs

ASFLAGS = -g -msoft-float

CFLAGS = -g -c -O3 -flto -pipe \
	-fomit-frame-pointer \
	-funroll-loops \
	-fno-stack-protector \
	-mno-check-zero-division \
	-msoft-float -nostdlib -mips1 -march=3000 -mtune=3000 \
	-Isrc -Wall -Wextra \
	-Wno-shift-negative-value \
	-Wno-unused-variable -Wno-unused-function -Wno-pointer-sign \

LDFLAGS = -g -O3 -flto -Wl,-T,link.ld -Wl,-Ttext-segment=0x8000F800 -pipe \
	-mtune=3000 -march=3000 \
	-fomit-frame-pointer \
	-fno-stack-protector \
	-funroll-loops \
	-mno-check-zero-division \
	\
	-msoft-float \
	-L/usr/local/mipsel-none-elf/lib/soft-float/

LIBS = -lm -lc -lgcc

# stuff omitted:
# O2:
# O3:
# -funswitch-loops - slows things down
# -fipa-cp-clone - also slows things down
#

EXE_NAME=boot
ISO_NAME=fromage

OBJDIR = obj
SRCDIR = src
INCLUDES = src/psx.h src/common.h

OBJS = $(OBJDIR)/head.o \
	\
	$(OBJDIR)/gui.o \
	$(OBJDIR)/gpu.o \
	$(OBJDIR)/joy.o \
	$(OBJDIR)/world.o \
	\
	$(OBJDIR)/main.o


all: $(EXE_NAME).exe $(ISO_NAME).cue

clean:
	$(RM_F) $(OBJS) $(OBJDIR)/$(EXE_NAME).elf $(ISO_NAME).bin $(ISO_NAME).cue tools/pscd-new

$(ISO_NAME).cue: $(ISO_NAME) tools/pscd-new
	./tools/pscd-new manifest.txt

tools/pscd-new: tools/pscd-new.c
	$(CC) -o tools/pscd-new tools/pscd-new.c

$(ISO_NAME): $(EXE_NAME).exe
	$(MKISOFS) -o $(ISO_NAME) system.cnf $(EXE_NAME).exe

$(EXE_NAME).exe: $(OBJDIR)/$(EXE_NAME).elf
	#$(CROSS_OBJCOPY) -O binary -j .text.head $(OBJDIR)/$(EXE_NAME).elf $(OBJDIR)/$(EXE_NAME).head
	$(CROSS_OBJCOPY) -O binary $(OBJDIR)/$(EXE_NAME).elf $(EXE_NAME).exe
	#$(CROSS_OBJCOPY) -O elf32-littlemips $(OBJDIR)/$(EXE_NAME).elf $(EXE_NAME).exe

$(OBJDIR)/$(EXE_NAME).elf: $(OBJS)
	$(CROSS_CC) -o $(OBJDIR)/$(EXE_NAME).elf $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CROSS_CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR)/head.o: $(SRCDIR)/head.S $(INCLUDES)
	$(CROSS_AS) -o $@ $(ASFLAGS) $<

