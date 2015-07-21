#include <system.h>
#include <ata.h>
#include <kmem.h>
#include <device.h>

void ata_wait()
{
	inb(ATA_CMD);
	inb(ATA_CMD);
	inb(ATA_CMD);
	inb(ATA_CMD);
}

void ata_read_sectors(uint32_t LBA, uint8_t sectors, void *buf)
{
	asm("pushfq");	// Storing rflags register
	outb(ATA_LBA3, 0xFF & ( (LBA >> 24) | ATA_DRV0 | ATA_LBA_MODE) );
	outb(0x1F1, 0);	// Wait !!
	outb(ATA_SECT, sectors);
	outb(ATA_LBA0, 0xFF & ( LBA >> 0  ) );
	outb(ATA_LBA1, 0xFF & ( LBA >> 8  ) );
	outb(ATA_LBA2, 0xFF & ( LBA >> 16 ) );
	
	outb(ATA_CMD,  0xFF & ATA_READ);
	
	uint8_t *_buf = buf;
	_buf -= 512;
	while(sectors--)
	{
		ata_wait();
		uint8_t status;
		do /*debug("status %x\n",*/ status = inb(ATA_CMD);	//);
		while( (status & 0x80) || !(status & 0x8));	// Wait until buffer is ready
	
		asm("movq %0, %%rdi; rep insw;"
			:
			:"g"( _buf += 512 ), "c"(256), "d"(ATA_DATA)
			:"rdi"
			);
	}	
	
	//debug("buf %lx\n", *(uint64_t*)buf);
	asm("popfq");	// Restoring rflags register
}

uint32_t ata_read(inode_t *currently_unused, uint32_t addr, uint32_t size, void *buf)
{
	/* TODO : Try to optimize this by doing some math 
			  instead of being dumb and copying the contents */
			  
	uint32_t LBA = addr/512;
	uint32_t offset = addr - 512 * LBA;
	uint32_t sectors = size/512 + + (size%512?1:0);
	uint8_t *_buf = kmalloc(512 * sectors);
	ata_read_sectors(LBA, sectors, _buf);
	memcpy(buf, _buf + offset, size);
	kfree(_buf);
	return size;
}

// TODO : this ata_write won't work because it's too fast
void ata_write(uint32_t LBA, uint8_t sectors, void *buf)
{
	asm("pushfq");	// Storing rflags register
	outb(ATA_LBA3, 0xFF & ( (LBA >> 24) | ATA_DRV0 | ATA_LBA_MODE) );
	outb(ATA_SECT, sectors);
	outb(ATA_LBA0, 0xFF & ( LBA >> 0  ) );
	outb(ATA_LBA1, 0xFF & ( LBA >> 8  ) );
	outb(ATA_LBA2, 0xFF & ( LBA >> 16 ) );
	
	outb(ATA_CMD, 0xFF & ATA_WRITE);
	ata_wait();
	while(!(inb(ATA_CMD)&8));	// Wait until buffer is ready
	asm("movq %0, %%rsi; rep outsw;"
		:
		:"g"(buf), "c"(256 * sectors), "d"(ATA_DATA)
		:"rsi"
		);
	asm("popfq");	// Restoring rflags register
}

dev_t atadev = 
	{
		.read = ata_read,
	};
