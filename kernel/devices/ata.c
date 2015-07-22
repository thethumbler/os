#include <system.h>
#include <kmem.h>
#include <device.h>
#include <devfs.h>
#include <ata.h>
#include <string.h>

#define MBR_TYPE_UNUSED             0x00
#define MBR_BOOT_SIGNATURE          0xAA55

#pragma pack(1)

typedef struct {
    uint8_t   status;
    struct {
        uint8_t   h;
        uint16_t  cs;
    } start_chs;
    uint8_t   type;
    struct {
        uint8_t   h;
        uint16_t  cs;
    } end_chs;
    uint32_t  start_lba;
    uint32_t  sectors_count;
} partition_t;

typedef struct {
    uint8_t              bootloader[440];	// Actual Bootloader code
    uint32_t             disk_signiture;
    uint16_t             copy_protected;
    partition_t          ptab[4];			// Partition table
    uint16_t             boot_signature;
} mbr_t;

#pragma pack()

mbr_t * read_mbr(inode_t *inode)
{
    static mbr_t mbr;
    if(inode->fs->read(inode, 0, sizeof(mbr_t), (void*)&mbr) != sizeof(mbr_t))
        return NULL;

    if(mbr.boot_signature != MBR_BOOT_SIGNATURE)
        return NULL;
    
    return &mbr;
}


static uint32_t ata_probe(inode_t *inode)
{
	// XXX : Seriously !? ... do you call this a driver !?
	static inode_t sda_inode =
	 	(inode_t) 
 		{
 			.name 	= "sda",
 			.type	= FS_BLKDEV,
 			.fs		= &devfs,
 			.dev	= &atadev,
 			.p		= NULL,
 		};
	vfs_create(inode, "/", &sda_inode);
 	
 	mbr_t *mbr_p = read_mbr(&sda_inode);

 	mbr_t mbr = *mbr_p;
 	int i;
    for(i = 0; i < 4; ++i){
        if(mbr.ptab[i].type == MBR_TYPE_UNUSED){
            continue;
        }
        atadev_private_t *p = kmalloc(sizeof(*p));
        *p = (atadev_private_t)
        {
        	.start_lba	= mbr.ptab[i].start_lba,
        	.end_lba	= mbr.ptab[i].start_lba + mbr.ptab[i].sectors_count - 1,
        };
        
        char *_name = "sda ";
        _name[3] = "1234"[i];
        
        inode_t *n = kmalloc(sizeof(*n));
        *n = (inode_t)
        {
        	.name	= strdup(_name),
        	.type	= FS_BLKDEV,
        	.fs		= &devfs,
        	.dev	= &atadev,
        	.p		= p,
        };
        vfs_create(inode, "/", n);
    }
}

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
		do status = inb(ATA_CMD);
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

static uint32_t
ata_read(inode_t *inode, uint64_t addr, uint64_t size, void *buf)
{
	debug("read %lx, %lx, %lx\n", addr, size, buf);
	/* TODO : Try to optimize this by doing some math 
			  instead of being dumb and copying the contents */
			  
	if(inode && inode->p)	// Partition !?
	{
		atadev_private_t *p = inode->p;
		return ata_read(NULL, p->start_lba * 512 + addr, size, buf);
	}
	
	uint32_t LBA = addr/512;
	uint32_t offset = addr - 512 * LBA;
	uint32_t sectors = size/512 + + (size%512?1:0);
	uint8_t *_buf = kmalloc(512 * sectors);
	ata_read_sectors(LBA, sectors, _buf);
	memcpy(buf, _buf + offset, size);
	kfree(_buf);
	debug("ret\n");
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
		.name  = "ATA",
		.probe = &ata_probe,
		.read  = &ata_read,
	};
