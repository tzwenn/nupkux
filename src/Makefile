#
# Makefile for the Nupkux kernel
# Copyright (C) 2007, 2008 Sven Köhler
# Use on your own risk.
#

PROJDIRS   = boot drivers fs include kernel lib mm
INCLUDEDIR = include 

SFILES   = $(shell find $(PROJDIRS) -name "*.S")
CFILES   = $(shell find $(PROJDIRS) -name "*.c")
SRCFILES = $(SFILES) $(CFILES)

OSFILES  = $(patsubst %.S,%.o,$(SFILES))
OCFILES	 = $(patsubst %.c,%.o,$(CFILES))
OBJFILES = $(OSFILES) $(OCFILES)

AUXFILES = COPYING Makefile link.ld

WFLAGS = -Wall -Werror -Wcast-align -Wwrite-strings -Wshadow -Winline -Wredundant-decls \
	 -Wstrict-prototypes -Wpointer-arith -Wnested-externs -Wno-long-long \
	 -Wunsafe-loop-optimizations

AS	= gcc
ASFLAGS	= -c

CC	= gcc
CFLAGS	= -c $(WFLAGS) -nostartfiles -nodefaultlibs -nostdlib -ffreestanding -fstrength-reduce \
	  -fomit-frame-pointer -finline-functions -I$(INCLUDEDIR)

LD	= ld
LDFLAGS	= -Tlink.ld

DEPDIR		= .deps
DEPFILE		= $(DEPDIR)/$*.d
DEPFILES	= $(patsubst %.c,$(DEPDIR)/%.d,$(CFILES))   
MAKEDEPEND	= mkdir -p $(DEPDIR)/$(*D); touch $(DEPFILE); makedepend -f $(DEPFILE) -- $(CFLAGS) -- $<

all:	$(OBJFILES) link

.S.o:
	@echo "  AS	  $@"
	@$(AS) $(ASFLAGS) -o $@ $<

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

distclean: 	clean
		@rm -f $(shell find . -name "*~")
		@rm -rf $(DEPDIR)
		
dist: 
	-@if [ ! -z $(DISTTMP) ]; then \
		for dir in $(PROJDIRS); do cp -axR $$dir $(DISTTMP)/$$dir; done; \
		for file in $(AUXFILES); do cp -axR $$file $(DISTTMP)/$$file; done; \
	fi

todolist:
	-@for file in $(SRCFILES); do grep -H TODO $$file; done; true

fixmelist:
	-@for file in $(SRCFILES); do grep -H FIXME $$file; done; true

