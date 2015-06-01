#ifndef _ATA_H
#define _ATA_H

#include <system.h>
#include <device.h>

#define ATA_LBA_MODE	0xE0
#define ATA_SECT		0x1F2	// number of sectors
#define ATA_LBA0		0x1F3	// bits 0 to 7
#define ATA_LBA1		0x1F4	// bits 8 to 15
#define ATA_LBA2		0x1F5	// bits 16 to 23
#define ATA_LBA3		0x1F6	// bits 24 to 27 & drive & mode
#define ATA_CMD			0x1F7	// command port	/ status register
#define ATA_DATA		0x1F0	// data port
#define ATA_READ		0x20	// LBA Read with retry command
#define ATA_WRITE		0x30	// LBA write with retry command
#define ATA_DRV0		0x00	// first drive
#define ATA_DRV1		0x10	// second drive

extern dev_t atadev;

#endif
