#
# Makefile for Nupkux Userspace Applications
# Copyright (C) 2008 Sven Köhler
# Use on your own risk.
#

PROGRAMS   =init utest hello getty sh login
INCLUDEDIR =include
PROJDIRS   = $(PROGRAMS) $(INCLUDEDIR) libc

WFLAGS = -Wall -Werror -Wcast-align -Wwrite-strings -Wshadow -Winline -Wredundant-decls \
	 -Wstrict-prototypes -Wpointer-arith -Wnested-externs -Wno-long-long \
	 -Wunsafe-loop-optimizations

AS	= gcc
ASFLAGS	= -c

CC	= gcc
CFLAGS	= -c $(WFLAGS) -nostartfiles -nodefaultlibs -nostdlib -ffreestanding -I$(INCLUDEDIR)

LD	= ld
LDFLAGS	=

LIBCFILES=$(shell find libc -name "*.c")
LIBCOBJS=$(patsubst %.c,%.o,$(LIBCFILES))

SRCFILES=$(shell find $(EXECUTABLE) -name "*.c")
OBJFILES=$(patsubst %.c,%.o,$(SRCFILES))

AUXFILES   = Makefile link.ld crt0.S

MAKE		= make

DEPDIR		= .deps
DEPFILE		= $(DEPDIR)/$*.d
DEPFILES	= $(patsubst %.c,$(DEPDIR)/%.d,$(SRCFILES) $(LIBCFILES))
MAKEDEPEND	= mkdir -p $(DEPDIR)/$(*D); touch $(DEPFILE); makedepend -f $(DEPFILE) -- $(CFLAGS) -- $<

OUTDIR  =bin
INSTDIR =../initrd/bin

.PHONY: all crt libc programs install $(PROGRAMS)

all:	crt libc programs
	
.S.o:
	@echo "  AS	  $@"
	@$(AS) $(ASFLAGS) -o $@ $<

.c.o:
	@echo "  CC	  $@"
	@$(MAKEDEPEND)
	@$(CC) $(CFLAGS) -o $@ $<

-include $(DEPFILES)

crt:	crt0.o
	@mkdir -p $(OUTDIR)

libc:	$(LIBCOBJS)

programs:
	-@for program in $(PROGRAMS); do ($(MAKE) -s "EXECUTABLE= $$program" $$program); done; true

install:
	@mkdir -p $(INSTDIR)
	@rm -f $(INSTDIR)/*
	@cp $(OUTDIR)/* $(INSTDIR)/

$(EXECUTABLE):	$(OBJFILES) link

link:
	@echo "  LD	  $(EXECUTABLE)"
	@($(LD) $(LDFLAGS) -o $(OUTDIR)/$(EXECUTABLE) crt0.o $(LIBCOBJS) $(OBJFILES))

clean:
	@echo "  CLEAN	  usr"
	@rm -f $(shell find . -name "*.o")
	@rm -rf $(OUTDIR)

distclean:	clean
		@rm -f $(shell find . -name "*~")
		@rm -rf $(DEPDIR)


