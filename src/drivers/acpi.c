/*
 *  Copyright (C) 2008 Sven Köhler
 *
 *  This file is part of Nupkux.
 *
 *  Nupkux is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Nupkux is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Nupkux.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <drivers/acpi.h>
#include <time.h>
#include <lib/memory.h>

#define TIME_TO_WAIT	300
#define DELAY		10

static UINT *SMI_CMD;
static char ACPI_ENABLE;
static char ACPI_DISABLE;
static UINT *PM1a_CNT;
static UINT *PM1b_CNT;
static USHORT SLP_TYPa;
static USHORT SLP_TYPb;
static USHORT SLP_EN;
static USHORT SCI_EN;
static char PM1_CNT_LEN;

static UINT *acpiCheckRSDPtr(UINT *ptr)
{
	const char *sig = "RSD PTR ";
	struct RSDPtr *rsdp = (struct RSDPtr *) ptr;
	char *bptr;
	char check = 0;
	int i;

	if (memcmp(sig, rsdp, 8)) return 0;
	bptr = (char *)ptr;
	for (i = 0; i < sizeof(struct RSDPtr); i++) {
		check += *bptr;
		bptr++;
	}
	if (!check) return (UINT *)rsdp->RsdtAddress;
	else return 0;
}

static UINT *acpiGetRSDPtr(void)
{
	UINT *addr;
	UINT *rsdp;

	for (addr = (UINT *)0x000E0000; (int)addr < 0x00100000; addr += 0x10 / sizeof(addr)) {
		rsdp = acpiCheckRSDPtr(addr);
		if (rsdp) return rsdp;
	}
	int ebda = *((short *)0x40E);
	ebda = (ebda * 0x10) & 0x000FFFFF;
	for (addr = (UINT *)ebda; (int) addr < ebda + 1024; addr += 0x10 / sizeof(addr)) {
		rsdp = acpiCheckRSDPtr(addr);
		if (rsdp) return rsdp;
	}
	return 0;
}

static int acpiCheckHeader(UINT *ptr, const char *sig)
{
	if (!memcmp(ptr, sig, 4)) {
		char *checkPtr = (char *)ptr;
		int len = *(ptr + 1);
		char check = 0;
		while (len--) {
			check += *checkPtr;
			checkPtr++;
		}
		if (!check) return 0;
	}
	return -1;
}

static int acpiEnable(void)
{
	if (inportw((UINT)PM1a_CNT)&SCI_EN) return 0;
	if (!(SMI_CMD && ACPI_ENABLE)) return -1;
	outportb((UINT)SMI_CMD, ACPI_ENABLE);
	int i;
	for (i = 0; i < TIME_TO_WAIT; i++) {
		if ((inportw((UINT) PM1a_CNT)&SCI_EN) == 1) break;
		sleep(DELAY);
	}
	if (PM1b_CNT)
		for (; i < TIME_TO_WAIT; i++) {
			if ((inportw((UINT) PM1b_CNT) &SCI_EN) == 1) break;
			sleep(DELAY);
		}
	if (i < TIME_TO_WAIT) return 0;
	else return -1;
}

int setup_ACPI(void)
{
	UINT *ptr = acpiGetRSDPtr();
	if (!ptr || acpiCheckHeader(ptr, "RSDT")) return -1;
	int entrys = *(ptr + 1);
	entrys = (entrys - 36) / 4;
	ptr += 9;
	while (entrys--) {
		if (!acpiCheckHeader((UINT *)*ptr, "FACP")) {
			entrys = -2;
			struct FACP *facp = (struct FACP *) * ptr;
			if (!acpiCheckHeader((UINT *)facp->DSDT, "DSDT")) {
				char *S5Addr = (char *)facp->DSDT + 36;
				int dsdtLength = *(facp->DSDT + 1) - 36;
				while (dsdtLength--) {
					if (!memcmp(S5Addr, "_S5_", 4)) break;
					S5Addr++;
				}
				if ((dsdtLength > 0) && ((*(S5Addr - 1) == 0x08 || (*(S5Addr - 2) == 0x08 && *(S5Addr - 1) == '\\')) && *(S5Addr + 4) == 0x12)) {
					S5Addr += 5;
					S5Addr += ((*S5Addr & 0xC0) >> 6) + 2;
					if (*S5Addr == 0x0A) S5Addr++;
					SLP_TYPa = *(S5Addr) << 10;
					S5Addr++;
					if (*S5Addr == 0x0A) S5Addr++;
					SLP_TYPb = *(S5Addr) << 10;
					SMI_CMD = facp->SMI_CMD;
					ACPI_ENABLE = facp->ACPI_ENABLE;
					ACPI_DISABLE = facp->ACPI_DISABLE;
					PM1a_CNT = facp->PM1a_CNT_BLK;
					PM1b_CNT = facp->PM1b_CNT_BLK;
					PM1_CNT_LEN = facp->PM1_CNT_LEN;
					SLP_EN = 1 << 13;
					SCI_EN = 1;
					return 0;
				}
			}
		}
		ptr++;
	}
	return -1;
}

void acpiPowerOff(void)
{
	if (!SCI_EN) return;
	acpiEnable();
	outportw((UINT)PM1a_CNT, SLP_TYPa | SLP_EN);
	if (PM1b_CNT)outportw((UINT)PM1b_CNT, SLP_TYPb | SLP_EN);
}
