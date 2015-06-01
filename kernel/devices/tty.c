#include <system.h>
#include <kmem.h>
#include <vfs.h>
#include <string.h>
#include <debug.h>
#include <tty.h>
#include <kbd.h>
#include <scheduler.h>
#include <isr.h>
#include <process.h>

static tty_count = 0;
static tty_master_set = 0;
inode_t *ttym;

typedef struct
{
	uint8_t *vmem;			// Video Memory
	uint32_t cur_tty;		// Currently active tty
} tty_master_device_t;

static void tty_load_master(void *ptr)
{
	tty_master_set = 1;
	inode_t *tty = kmalloc(sizeof(inode_t));
	ttym = tty;
	tty->name = "ttym";
	tty->type = FS_FILE;
	tty->fs   = &devtty;
	tty_device_t *tmp = kmalloc(sizeof(tty_device_t));
	tmp->is_master = 1;
	tty_master_device_t *tmp2 = kmalloc(sizeof(tty_master_device_t));
	tmp2->vmem = ptr;
	tmp2->cur_tty = 0;
	tmp->master = tmp2;
	tty->p = tmp;
	vfs_create(vfs_root, "/dev/", tty);
}

void tty_switch(uint32_t tty)
{
	((tty_master_device_t*)((tty_device_t*)ttym->p)->master)->cur_tty = tty;
	uint8_t *v = strcat("/dev/tty", (uint64_t)itoa(tty));
	inode_t *f = vfs_trace_path(vfs_root, v);
	
	uint8_t *vmem = ((tty_master_device_t*)((tty_device_t*)ttym->p)->master)->vmem;
	uint64_t i;
	for(i = 0; i < f->size; i+=2)
		vmem[i] = ((tty_device_t*)f->p)->ptr[i];
	
	outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)((((tty_device_t*)f->p)->pos)/2 & 0xFF));

    outb(0x3D4, 0x0E);
   	outb(0x3D5, (uint8_t)((((tty_device_t*)f->p)->pos)/2 >> 8) & 0xFF);
}

static void tty_load(void *ptr)
{
	if(!tty_master_set) return tty_load_master(ptr);
	inode_t *tty = kmalloc(sizeof(inode_t));
	tty->name = strcat("tty", (uint64_t)itoa(tty_count));
	tty->type = FS_FILE;
	tty->fs = &devtty;
	tty_device_t *tmp = kmalloc(sizeof(tty_device_t));

	tmp->is_master = 0;
	tmp->id = tty_count++;
	tmp->ptr = ptr;
	tmp->pos = 0;
	tmp->row = 0;
	tmp->col = 0;
	
	tty->p = tmp;
	tty->size = 25 * 80 * 2;
	vfs_create(vfs_root, "/dev/", tty);
}

static void tty_scroll(uint8_t *ptr, uint32_t n)
{
	uint32_t i = 0, j = n*2*80;
	
	while(j < 80*25*2)
	{
		ptr[i] = ptr[j];
		ptr[j] = 0;
		i += 2;
		j += 2;
	}

}

static void tty_write(inode_t *inode, void *_buf, uint32_t size)
{
	if(((tty_device_t*)inode->p)->is_master) return tty_switch((uint32_t)_buf);
	uint8_t *buf = _buf;
	uint8_t *ptr = ((tty_device_t*)inode->p)->ptr;
	uint32_t *pos = &((tty_device_t*)inode->p)->pos;
	uint8_t dum = 0;
	while(size--)
	{
		repeat:
		if(!*buf) goto done;
		if(*pos < inode->size)
		{
			switch(*buf)
			{
				case '\n':
					dum = 1;
					*pos += 162 - *pos%160; 
					++buf; 
					if( *pos >= inode->size )	// last line !?
					{	
						tty_scroll(ptr, 1);
						*pos -= 160;
					}
					break;
				case '\b': 
					*pos -= 2;
					*(ptr + *pos) = ' ';
					++buf; 
					break;
				default:
					*(ptr + *pos) = *buf++;
					*pos += 2;
			}
		} 
		else
		{
			tty_scroll(ptr, 1);
			*pos -= 160;
			--buf;
			goto repeat;
		}
	}
	done:
	dum?(*pos -= 2):0;
	

	if((uint64_t)((tty_master_device_t*)((tty_device_t*)ttym->p)->master)->cur_tty 
		== (uint64_t)((tty_device_t*)inode->p)->id)
	{
		uint8_t *vmem = ((tty_master_device_t*)((tty_device_t*)ttym->p)->master)->vmem;
		uint64_t i;
		for(i = 0; i < inode->size; i+=2)
			vmem[i] = ((tty_device_t*)inode->p)->ptr[i];
			
    	outb(0x3D4, 0x0F);
    	outb(0x3D5, (uint8_t)(*pos/2 & 0xFF));

	    outb(0x3D4, 0x0E);
    	outb(0x3D5, (uint8_t)((*pos/2 >> 8) & 0xFF));
    }
}

static file_t *tty_open(inode_t *i)
{
	file_t *ret = kmalloc(sizeof(file_t));
	ret->buf = ((tty_device_t*)i->p)->ptr;
	ret->pos = ((tty_device_t*)i->p)->pos;
	ret->size = i->size;
	return ret;
}

#define ESC		0x01
#define SHIFT	0x2A
#define _SHIFT	0xAA
#define CAPS	3
#define LALT	0x38
#define _LALT	0xB8
#define F1		0x3B

uint8_t kbd_us[] = 
{
	'\0', ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	'\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
	'\0', '\0', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	'\0', '\0', '\0', ' ',
};

uint8_t kbd_us_shift[] = 
{
	'\0', ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
	'\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"',
	'\0', '\0', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
};

static uint8_t shift = 0, alt = 0;
static uint8_t flush = 0;
//static uint8_t *_buf;
static uint32_t i = 0;
//static uint32_t _len = 0;
//static process_t *p;

void tty_kbd(uint8_t scancode)
{

	uint64_t cur_tty_id = (uint64_t)((tty_master_device_t*)((tty_device_t*)ttym->p)->master)->cur_tty;
	uint8_t *cur_tty_str = strcat("/dev/tty", itoa(cur_tty_id));
	debug("cur_tty %s\n", cur_tty_str);
	inode_t *cur_tty = vfs_trace_path(vfs_root, cur_tty_str);
	tty_device_t *_tty = cur_tty->p;
	uint64_t cur_pdpt = 0;
	if(_tty->p) cur_pdpt = switch_pdpt(_tty->p->pdpt);
	
	uint8_t buf[2];

	if(scancode == SHIFT)   { shift = 1; goto done; }
	if(scancode == _SHIFT)  { shift = 0; goto done; }
	if(scancode == LALT)	{ alt = 1; goto done; }
	if(scancode == _LALT)	{ alt = 0; goto done; }
	
	extern void tty_switch(uint32_t);
	// Special keys
	if(alt && scancode == ESC) 
	{
		//tty_switch(0);
		vfs_write(vfs_trace_path(vfs_root, "/dev/ttym"), 0, 0);
		goto done;
	}
		
	if(alt && scancode == F1)
	{
		tty_switch(1);
		goto done;
		//vfs_write(vfs_trace_path(vfs_root, "/dev/ttym"), 1, 0);
	}
	
	if(scancode < 60) 
	{
		if(shift) buf[0] = kbd_us_shift[scancode];
		else buf[0] = kbd_us[scancode];
		buf[1] = '\0';
		tty_write(cur_tty, buf, 1);
		(i < _tty->len)?(_tty->buf[i++] = buf[0]):(_tty->buf[i] = '\0');
		if(kbd_us[scancode] == '\n') 
		{
				if(_tty->buf)_tty->buf[i] = '\0';
				i = 0;
				if(_tty->p)
					_tty->p->status = READY;
				_tty->p = NULL;
				_tty->buf = NULL;
				_tty->len = 0;
		}
	}
	
	done:
	if(cur_pdpt) switch_pdpt(cur_pdpt);
	return;
}

void tty_read(inode_t *tty, void *buf, uint64_t len)
{
	tty_device_t *_tty = tty->p;
	if(!_tty->p) _tty->p = current_process;
	_tty->p->status = WAITING;
	_tty->buf = buf;
	_tty->len = len;
}

fs_t devtty = 
	(fs_t)
	{
		.name = "TTY",
		.load = &tty_load,
		.open = &tty_open,
		.read = &tty_read,
		.write = &tty_write,
	};
