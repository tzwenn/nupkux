#
# Makefile for the Squaros kernel
# Copyright (c) 2007 Sven Köhler
#

AS	=as
ASINT	=nasm
ASINTFLAGS =-f aout

CC	=gcc
CFLAGS	=-c -Wall -nostartfiles -nodefaultlibs -nostdlib -ffreestanding -Iinclude

LD	=ld
LDFLAGS	=-T link.ld

PROJDIRS=kernel lib mm

SRCFILES = $(shell find $(PROJDIRS) -mindepth 1 -maxdepth 4 -name "*.c")
HDRFILES = $(shell find include -mindepth 1 -maxdepth 4 -name "*.h")
OBJFILES = $(patsubst %.c,%.c.o,$(SRCFILES))
DEPFILES = $(HDRFILES)
PRJFILES = $(HDRFILES) $(SRCFILES) boot/dts.asm boot/loader.s Makefile link.ld NoteToMe squaros
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

link:	
	@echo "Link ..."	
	@($(LD) $(LDFLAGS) -o squaros boot/loader.o boot/dts.o $(OBJFILES))

clean: 
	@echo "Clean up ..."
	-@if [ -f boot/loader.o ]; then rm boot/loader.o; fi
	-@if [ -f boot/dts.o ]; then rm boot/dts.o; fi
	-@for file in $(OBJFILES); do if [ -f $$file ]; then rm $$file; fi; done

backup: clean
	@echo "Copy development directory ..."
	@cp -R . $(BACKUPTMP)
	@for file in $(PRJFILES); do if [ -f $(BACKUPTMP)/$$file~ ]; then rm $(BACKUPTMP)/$$file~; fi; done
	@echo "Make archive ..."
	@(cd $(BACKUPTMP); tar -cf squaros.tar *; gzip squaros.tar)
	@echo "Save archive ..."
	@cp $(BACKUPTMP)/squaros.tar.gz $(BACKUPDIR)/squaros-$(shell date +%y-%m-%d).tar.gz
	@echo "Remove temporary files ..."
	@rm -r $(BACKUPTMP)
