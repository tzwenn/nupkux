#
# Makefile for Nupkux Tools
# Copyright (C) 2008 Sven Köhler
# Use on your own risk.
#

TOOLS	= mkinitrd

CC	= gcc
CFLAGS	= -Wall -Werror

MAKE	= make

SRCFILES=$(shell find $(EXECUTABLE) -name "*.c")

.PHONY: clean $(TOOLS)

all:
	-@for tool in $(TOOLS); do ($(MAKE) -s "EXECUTABLE= $$tool" $$tool); done; true

$(EXECUTABLE):
	@echo "  CC [LD]  $(EXECUTABLE)"
	$(CC) $(CFLAGS) $(SRCFILES) -o $(EXECUTABLE)/$(EXECUTABLE)

clean:
	@echo "  CLEAN	  tools"
	-@for tool in $(TOOLS); do rm -f $$tool/$$tool; done; true

distclean:	clean
		@rm -f $(shell find . -name "*~")
