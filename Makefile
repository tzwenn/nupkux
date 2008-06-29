#
# Makefile for the Nupkux kernel
# Copyright (C) 2007, 2008 Sven Köhler
#

PROJDIRS   =kernel lib mm fs
INCLUDEDIR =include 
MAXDEPTH   =5

AS	=as
ASINT	=nasm
ASINTFLAGS =-f aout

CC	=gcc
CFLAGS	=-c -Wall -nostartfiles -nodefaultlibs -nostdlib -ffreestanding -I$(INCLUDEDIR)

LD	=ld
LDFLAGS	=-T link.ld

SRCFILES = $(shell find $(PROJDIRS) -mindepth 1 -maxdepth $(MAXDEPTH) -name "*.c")
HDRFILES = $(shell find $(INCLUDEDIR) -mindepth 1 -maxdepth $(MAXDEPTH) -name "*.h")
OBJFILES = $(patsubst %.c,%.c.o,$(SRCFILES))
DEPFILES = $(HDRFILES)
PRJFILES = $(HDRFILES) $(SRCFILES) boot/dts.asm boot/loader.s boot/process.asm Makefile link.ld COPYING
BACKUPTMP = ../backup-tmp
BACKUPDIR = ../backups

.PHONY: boot kernel

all:	kernel boot link clean
	@echo "Finished ..."

kernel: 
	@echo "Start compilation ..."
	-@for file in $(SRCFILES); do $(CC) $$file $(CFLAGS) -o $$file.o; done; true

boot:
	@echo "Assemble ..."
	@($(AS) -o boot/loader.o boot/loader.s)
	@($(ASINT) $(ASINTFLAGS) -o boot/dts.o boot/dts.asm)
	@($(ASINT) $(ASINTFLAGS) -o boot/process.o boot/process.asm)

link:	
	@echo "Link ..."	
	@($(LD) $(LDFLAGS) -o nupkux boot/loader.o boot/dts.o boot/process.o $(OBJFILES))

clean: 
	@echo "Clean up ..."
	-@if [ -f boot/loader.o ]; then rm boot/loader.o; fi
	-@if [ -f boot/dts.o ]; then rm boot/dts.o; fi
	-@if [ -f boot/process.o ]; then rm boot/process.o; fi
	-@for file in $(OBJFILES); do if [ -f $$file ]; then rm $$file; fi; done

backup: clean
	@echo "Copy development directory ..."
	@cp -axR . $(BACKUPTMP)
	@for file in $(PRJFILES); do if [ -f $(BACKUPTMP)/$$file~ ]; then rm $(BACKUPTMP)/$$file~; fi; done
	@echo "Make archive ..."
	@(cd $(BACKUPTMP); tar -cf nupkux.tar *; gzip nupkux.tar)
	@echo "Save archive ..."
	@cp $(BACKUPTMP)/nupkux.tar.gz $(BACKUPDIR)/nupkux-snp$(shell date +%y%m%d).tar.gz
	@echo "Remove temporary files ..."
	@rm -r $(BACKUPTMP)
