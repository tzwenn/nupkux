#
# Makefile for the Nupkux kernel
# Copyright (C) 2007, 2008 Sven Köhler
# Use on your own risk.
#

PROJDIRS   = boot drivers fs include kernel lib mm
INCLUDEDIR = include 

AFILES   = $(shell find $(PROJDIRS) -name "*.s")
SFILES   = $(shell find $(PROJDIRS) -name "*.S")
CFILES   = $(shell find $(PROJDIRS) -name "*.c")
SRCFILES = $(AFILES) $(SFILES) $(CFILES)

OAFILES  = $(patsubst %.s,%.o,$(AFILES))
OSFILES  = $(patsubst %.S,%.o,$(SFILES))
OCFILES	 = $(patsubst %.c,%.o,$(CFILES))
OBJFILES = $(OAFILES) $(OSFILES) $(OCFILES)

AUXFILES = COPYING Makefile link.ld

WFLAGS = -Wall -Werror -Wcast-align -Wwrite-strings -Wshadow -Winline -Wredundant-decls \
	 -Wstrict-prototypes -Wpointer-arith -Wnested-externs -Wno-long-long \
	 -Wunsafe-loop-optimizations

AS	= as

ASINT	= nasm
ASINTFLAGS = -f aout

CC	= gcc
CFLAGS	= -c $(WFLAGS) -nostartfiles -nodefaultlibs -nostdlib -ffreestanding -I$(INCLUDEDIR)

LD	= ld
LDFLAGS	= -Tlink.ld

DEPDIR		= .deps
DEPFILE		= $(DEPDIR)/$*.d
DEPFILES	= $(patsubst %.c,$(DEPDIR)/%.d,$(CFILES))
MAKEDEPEND	= mkdir -p $(DEPDIR)/$(*D); rm -f $(DEPFILE); touch $(DEPFILE); makedepend -f $(DEPFILE) -- $(CFLAGS) -- $<

all:	$(OBJFILES) link

.s.o:
	@echo "  AS	  $@"
	@$(AS) -o $@ $<

.S.o:
	@echo "  AS	  $@"
	@$(ASINT) $(ASINTFLAGS) -o $@ $<

.c.o:	
	@echo "  CC	  $@"
	@$(MAKEDEPEND)
	@$(CC) $(CFLAGS) -o $@ $<

-include $(DEPFILES)

link:	
	@echo "  LD	  nupkux"
	@$(LD) $(LDFLAGS) -o nupkux $(OBJFILES)

clean: 
	@echo "  CLEAN	  src"
	@rm -f $(shell find $(PROJDIRS) -name "*.o")
	-@if [ -f nupkux ]; then rm -f nupkux; fi

distclean: clean
		@rm -f $(shell find $(PROJDIRS) -name "*~")
		@rm -rf $(DEPDIR)
		
dist: 
	-@if [ ! -z $(DISTTMP) ]; then \
		for dir in $(PROJDIRS); do cp -axR $$dir $(DISTTMP)/$$dir; done; \
		for file in $(AUXFILES); do cp -axR $$file $(DISTTMP)/$$file; done; \
	fi

todolist:
	-@for file in $(SRCFILES); do grep -H TODO $$file; done; true

