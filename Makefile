#
# Makefile for Nupkux Distribution
# Copyright (C) 2008 Sven Köhler 
# Use on your own risk!
#

KERNELSOURCE	= src
USERSOURCE	= usr
TOOLSOURCE	= tools

MOUNTPOINT	= bootdisk
INITRDDIR	= initrd

FLOPPYIMAGE	= bootdisk.img
FSTYPE		= ext2
KERNELIMAGE	= nupkux
MAKEINITRD	= $(TOOLSOURCE)/mkinitrd/mkinitrd

AUXFILES	= Makefile
PRJDIRS		= $(KERNELSOURCE) $(USERSOURCE) $(TOOLSOURCE)

MAKE		= make
CPFLAGS		= -Rax
DISTTMP		= .dist_tmp
TARFLAGS	= -z
DISTNAME	= nupkux-dist.tar.gz

REALINST	= NO

SUDO		= sudo

.PHONY: install tools help

all: kernel userspace tools

install: do_mount do_initrd do_install do_umount

help:
	@echo "Makefile for Nupkux Distribution"
	@echo ""
	@echo "TARGETS:"
	@echo "   kernel	Kernel Image"
	@echo "   userspace	Userspace Applications"
	@echo "   tools	Auxillary Programms on building system"
	@echo "   all		Kernel, Userspace and Tools"
	@echo "   install	Install Nupkux on virtual floppy drive"
	@echo "   clean	Remove compiled binaries and objects"
	@echo "   distclean	Remove as \"clean\" and backup files too"
	@echo "   dist		Create a tarball containing all sources"
	@echo "   run_qemu	Run bootdisk-Image in QEMU"
	@echo "   help		Show this message"
	@echo ""
	@echo "NOTES:"
	@echo "  1. To install Nupkux on your real drive too, run make with \"REALINST=YES\""
	@echo "  2. If any error happens during install, the virtual drive won't be unmounted."
	@echo "     In this case run: make do_umount"
	@echo "  3. Any install operation needs superuser rights."
	@echo "  4. The bootdisk-Image is NOT in the standard distribution."
		

kernel:
	@echo "==========Build Kernel==========="
	@$(MAKE) -sC $(KERNELSOURCE)

userspace:
	@echo "====Build Userspace programms===="
	@$(MAKE) -sC $(USERSOURCE)

tools:
	@echo "===========Build Tools==========="
	@$(MAKE) -sC $(TOOLSOURCE)

do_mount:
	@echo "===Mount virtual floppy drive===="
	@mkdir -p $(MOUNTPOINT)
	@$(SUDO) mount -t $(FSTYPE) $(FLOPPYIMAGE) $(MOUNTPOINT) -o loop

do_initrd:
	@echo "======Build initial ramdisk======"
	@mkdir -p $(INITRDDIR)/dev
	@$(MAKE) -sC $(USERSOURCE) install
	-@$(SUDO) -s $(MAKEINITRD) $(INITRDDIR) > $(MOUNTPOINT)/initrd 2> /dev/null

do_install:
	@echo "=Install Nupkux on virtual drive="
	@$(SUDO) cp $(KERNELSOURCE)/$(KERNELIMAGE) $(MOUNTPOINT)/nupkux
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!#
#                                                #
# The following two statements install nupkux on #
# your real drive. If you like trouble, run:     #
#                                                #
# # make "REALINST=YES" install                  #
#                                                #
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!#
	@if [ $(REALINST) = YES ]; then \
		$(SUDO) cp $(MOUNTPOINT)/nupkux /boot/nupkux; \
		$(SUDO) cp $(MOUNTPOINT)/initrd /boot/nupkux-initrd; \
	fi	

do_umount:
	@$(SUDO) umount $(MOUNTPOINT)
	@rmdir $(MOUNTPOINT)
	@echo "============Finished============="

run_qemu:
	@qemu -fda $(FLOPPYIMAGE) -boot a

clean:
	@$(MAKE) -sC $(KERNELSOURCE) clean
	@$(MAKE) -sC $(USERSOURCE) clean
	@$(MAKE) -sC $(TOOLSOURCE) clean

distclean:
	@$(MAKE) -sC $(KERNELSOURCE) distclean
	@$(MAKE) -sC $(USERSOURCE) distclean
	@$(MAKE) -sC $(TOOLSOURCE) distclean

dist:	distclean
	@echo "  AR	  $(DISTNAME)"
	@git archive HEAD | gzip > $(DISTNAME)

