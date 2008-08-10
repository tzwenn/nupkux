#
# Makefile for the Nupkux kernel
# Copyright (C) 2007, 2008 Sven Köhler
#

PROJDIRS   =boot kernel mm drivers fs lib
INCLUDEDIR =include 

AS	=as

ASINT	=nasm
ASINTFLAGS =-f aout

CC	=gcc
CFLAGS	=-c -Wall -Werror -nostartfiles -nodefaultlibs -nostdlib -ffreestanding -I$(INCLUDEDIR)

LD	=ld
LDFLAGS	=-T link.ld

AFILES = $(shell find $(PROJDIRS) -name "*.s")
SFILES = $(shell find $(PROJDIRS) -name "*.S")
CFILES = $(shell find $(PROJDIRS) -name "*.c")
SRCFILES = $(AFILES) $(SFILES) $(CFILES)
O1FILES = $(patsubst %.s,%.o,$(SRCFILES))
O2FILES = $(patsubst %.S,%.o,$(O1FILES))
OBJFILES = $(patsubst %.c,%.o,$(O2FILES))
BACKUPTMP = ../backup-tmp
BACKUPDIR = ../backups

all:	$(OBJFILES) link
	
.s.o:
	@echo "  AS	  $@"
	@$(AS) -o $@ $<

.S.o:
	@echo "  AS	  $@"
	@$(ASINT) $(ASINTFLAGS) -o $@ $<

.c.o:
	@echo "  CC	  $@"
	@$(CC) $(CFLAGS) -o $@ $<

link:	
	@echo "  LD	  nupkux"
	@($(LD) $(LDFLAGS) -o nupkux $(OBJFILES))

clean: 
	@echo "  CLEAN	  src"
	-@for file in $(OBJFILES); do if [ -f $$file ]; then rm -f $$file; fi; done; true
	-@if [ -f nupkux ]; then rm -f nupkux; fi

backup: clean
	@echo "Copy development directory ..."
	@cp -axR . $(BACKUPTMP)
	@for file in $(shell find -name "*~"); do if [ -f $(BACKUPTMP)/$$file ]; then rm $(BACKUPTMP)/$$file; fi; done
	@echo "Make archive ..."
	@(cd $(BACKUPTMP); tar -cf nupkux.tar *; gzip nupkux.tar)
	@echo "Save archive ..."
	@cp $(BACKUPTMP)/nupkux.tar.gz $(BACKUPDIR)/nupkux-snp$(shell date +%y%m%d).tar.gz
	@echo "Remove temporary files ..."
	@rm -r $(BACKUPTMP)

