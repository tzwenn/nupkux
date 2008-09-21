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

#ifndef _ACPI_H
#define _ACPI_H

#include <drivers/drivers.h>

struct RSDPtr
{
	char Signature[8];
	char CheckSum;
	char OemID[6];
	char Revision;
	UINT *RsdtAddress;
};

struct FACP
{
	char Signature[4];
	UINT Length;
	char unneded1[32];
	UINT *DSDT;
	char unneded2[4];
	UINT *SMI_CMD;
	char ACPI_ENABLE;
	char ACPI_DISABLE;
	char unneded3[10];
	UINT *PM1a_CNT_BLK;
	UINT *PM1b_CNT_BLK;
	char unneded4[17];
	char PM1_CNT_LEN;
};

extern void acpiPowerOff(void);
extern int setup_ACPI(void);

#endif
