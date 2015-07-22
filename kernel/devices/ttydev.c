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
#include <va_list.h>
#include <console.h>
#include <devfs.h>

static uint32_t tty_count = 0;
inode_t *cur_tty;

typedef struct
{
	uint8_t *vmem;			// Video Memory
	uint32_t cur_tty;		// Currently active tty
} tty_master_device_t;

static void tty_load_master(void *ptr)
{/*
//	inode_t *tty = kmalloc(sizeof(inode_t));
//	ttym = tty;
//	tty->name = "ttym";
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
*/
}

void tty_switch(uint32_t tty_id)
{
/*	uint8_t *tty_id_str = itoa(tty_id);
	uint8_t *tty_path = strcat("/dev/tty", tty_id_str);
	
	inode_t *inode = vfs_trace_path(vfs_root, tty_path);
	if(!inode) return;
	
	kfree(tty_id_str); kfree(tty_path);
	
	tty_device_t *tty = inode->p;
	tty->master->invoke(tty, TTYM_SWITCH);
/*	//((tty_master_t*)((tty_device_t*)ttym->p)->master)->cur_tty = tty;
	
	uint8_t *v = strcat("/dev/tty", (uint8_t*)itoa(tty));
	inode_t *f = vfs_trace_path(vfs_root, v);
	
	uint8_t *vmem = ((tty_master_device_t*)((tty_device_t*)ttym->p)->master)->vmem;
	uint64_t i;
	for(i = 0; i < f->size; i+=2)
		vmem[i] = ((tty_device_t*)f->p)->ptr[i];
	
	outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)((((tty_device_t*)f->p)->pos)/2 & 0xFF));

    outb(0x3D4, 0x0E);
   	outb(0x3D5, (uint8_t)((((tty_device_t*)f->p)->pos)/2 >> 8) & 0xFF);
*/
}

static inode_t *tty_load(void *ptr)
{
/*	inode_t *tty = kmalloc(sizeof(inode_t));
	tty->name = strcat("tty", (uint8_t*)itoa(tty_count));
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
	return tty;
*/
}


#define VIRTCON_SIZE(virtcon) ((virtcon->w) * (virtcon->h))
#define DIV_ROUND_UP(val, div) ((val/div) + !!(val%div));

static void tty_scroll(tty_device_t *tty, uint32_t n)
{
/*	uint32_t i = 0, j = n * tty->cols;
	uint8_t *ptr = tty->ptr;
	memcpy(tty->ptr, tty->ptr + n * tty->cols, VIRTCON_SIZE(virtcon) - n * tty->cols);
*/
}

static uint32_t
ttydev_probe(inode_t *inode)
{
	static tty_master_t ttym;
	ttym = 
		(tty_master_t)
		{
			.cur_tty	= 0,
			//.invoke		= ttym_invoke,
			.console	= vfs_trace_path(inode, "/console"),
		};

#if _DBG_CON_
	extern tty_device_t serial_tty_dev;
	static tty_device_t tty_dev;
	tty_dev = serial_tty_dev;
	tty_dev.master = &ttym;
#else
	static tty_device_t tty_dev;
	tty_dev = 
		(tty_device_t)
		{
			.id			= 0,
			.master		= &ttym,
			.pos		= 0,
			.virtcon	= get_virtcon(&tty_dev, 80, 25),
			.p			= NULL,
			.buf		= NULL,
			.len		= 0,
		};
#endif
	
	static inode_t tty_inode = 
		(inode_t)
		{
			.name	= "0",
			.type	= FS_CHRDEV,
			.fs		= &devfs,
			.dev	= &ttydev,
			.p		= &tty_dev,
		};
	
	static inode_t tty_dir = 
		(inode_t)
		{
			.name	= "tty",
			.type	= FS_DIR,
			//.fs	= &devfs,
			//.dev	= &ramdev,
		};

	cur_tty = &tty_inode;
		
	vfs_create(inode, "/", &tty_dir);
	vfs_create(&tty_dir, "/", &tty_inode);
}

static uint32_t 
ttydev_write(inode_t *inode, uint64_t offset_unused, uint64_t size, void *__buf)
{	
	uint8_t *_buf = __buf;
	tty_device_t *tty = inode->p;
	virtcon_device_t *virtcon = tty->virtcon;

	if((tty->pos + size) > VIRTCON_SIZE(virtcon))
	{
		uint32_t lines_to_scroll = 
			DIV_ROUND_UP(((tty->pos + size) - VIRTCON_SIZE(virtcon)), virtcon->w);
		virtcon->scroll(virtcon, lines_to_scroll);
		tty->pos -= lines_to_scroll * virtcon->w;
	}

	static uint32_t fg = 0x7, bg = 0, light = 0;
	uint32_t i = 0;
	uint32_t pos = 0;
	
	uint8_t *buf = kmalloc(size);
	memcpy(buf, _buf, size);
	
	while(i < size)
	{
		// Add special chararcters here
		while(i < size && buf[i] && !within(buf[i], "\n\b\033\t"))
			++i;

		switch(buf[i])
		{
			case '\033':
				buf[i] = '\0';
				virtcon->puts(	virtcon,
								&buf[pos], 
								i - pos,	// Size
								tty->pos % virtcon->w, // X
								tty->pos / virtcon->w, // Y
								fg, 	// fg colour
								bg		// bg color
							 );
				tty->pos += i - pos;
				pos = ++i;
				{
					// Parsing the escape sequence
					switch(buf[i])
					{
						// It'll get messy down there so we will drop `case'
						// indentation!
						
						case '[':
						{
							uint32_t k = i++;
							while(++k < size && buf[k] && !isalpha(buf[k]));
							if(buf[k])
							switch(buf[k])
							{
								case 'm':	// Switch color
								{
									uint8_t fg_c[] = {0, 4, 2, 6, 1, 5, 3, 7};
									uint32_t n = 0;
									while(i < k)
									{
										while(isdigit(buf[i]))
											n = 10 * n + buf[i++] - '0';
										
										if(n == 0)
										{
											fg = 0x7;
											bg = 0x0;
										} else
										if(n == 1)
										{
											fg |= 0x8;
										} else
										if(n == 2)
										{
											fg &= ~0x8;
										} else
										if(n >= 30 && n <= 37)
										{
											fg = fg & ~0x7 | fg_c[n - 30];
										} else
										if(n >= 90 && n <= 97)
										{
											fg = 0x8 | fg & ~0x7 | fg_c[n - 90];
										} else
										if(n >= 40 && n <= 47)
										{
											bg = bg & ~0x7 | fg_c[n - 40];
										}
										if(buf[i] != ';') break;
										++i;
										n = 0;
									}
									pos = ++i;
								}
								break;

								case 'H':
								{
									if(i == k)
									{
										virtcon->setcur(virtcon, 0, 0);
										tty->pos = 0;
										pos = ++i;
										break;
									}
									
									uint32_t c = 0, r = 0;
									while(isdigit(buf[i]))
										r = 10 * r + buf[i++] - '0';
									
									if(buf[i] != ';' && buf[i] != 'H')
										goto _default;
										
									++i;
									while(isdigit(buf[i]))
										c = 10 * c + buf[i++] - '0';

									if(buf[i] != 'H') goto _default;
									--c; --r;
									virtcon->setcur(virtcon, c, r);
									tty->pos = r * virtcon->w + c;
									pos = ++i;
								}
								break;
								
								case 'J':
								{
									// Now we only support 2J ( Erase all )
									if(i == k)
									{
										// Ignored
										break;
									}

									uint32_t n = 0;
									while(isdigit(buf[i]))
										n = 10 * n + buf[i++] - '0';

									if(buf[i] != 'J') goto _default;
									if(n == 2)
									{		
										uint32_t r = 0;
										char __buf[virtcon->w];
										memset(__buf, ' ', virtcon->w);

										while(r < virtcon->h)
											virtcon->puts(
															virtcon,
															__buf,
															virtcon->w,
															0,
															r++,
															fg,
															bg
														  );
										pos = ++i;
									}
								}
								break;
							}
						}
						default:
							_default:
							--i;
							break;
					}
				}
				break;
			
			case '\t':
				buf[i] = '\0';
				
				virtcon->puts(	virtcon,
								&buf[pos], 
								i - pos,	// Size
								tty->pos % virtcon->w, // X
								tty->pos / virtcon->w, // Y
								fg, 	// fg colour
								bg		// bg color
							 );
				
				tty->pos += i - pos + 4;
				pos = ++i;
				break;
				
			case '\n':
				buf[i] = '\0';
				if(tty->pos + virtcon->w >= VIRTCON_SIZE(virtcon))
				{
					virtcon->scroll(virtcon, 1);
					tty->pos -= virtcon->w;
				}
				
				virtcon->puts(	virtcon,
								&buf[pos], 
								i - pos,	// Size
								tty->pos % virtcon->w, // X
								tty->pos / virtcon->w, // Y
								fg, 	// fg colour
								bg		// bg color
							 );
				
				tty->pos += virtcon->w - tty->pos % virtcon->w;
				pos = ++i;
				break;

			case '\b':
				buf[i] = '\0';
				virtcon->puts(	virtcon,
								&buf[pos], 
								i - pos,	// Size
								tty->pos % virtcon->w, // X
								tty->pos / virtcon->w, // Y
								fg, 	// fg colour
								bg		// bg color
							 );

 				tty->pos += i - pos - 1;
 				
				virtcon->putc(	virtcon,
								'\0',
								tty->pos % virtcon->w, // X
								tty->pos / virtcon->w, // Y
								fg, 	// fg colour
								bg		// bg color
							 );
				pos = ++i;
				break;

			default:
				virtcon->puts(	virtcon,
								&buf[pos], 
								i - pos,	// Size
								tty->pos % virtcon->w, // X
								tty->pos / virtcon->w, // Y
								fg, 	// fg colour
								bg		// bg color
							 );
				tty->pos += i - pos;
				pos = ++i;
		}
	}
	
done:
	virtcon->setcur(virtcon, tty->pos%virtcon->w, tty->pos/virtcon->w);
	virtcon->draw(virtcon);
	return size;
}


#define ESC		0x01
#define  LSHIFT	0x2A
#define _LSHIFT	0xAA
#define  RSHIFT	0x36
#define _RSHIFT 0xB6
#define  CAPS	0x3A
#define _CAPS	0xBA
#define  LALT	0x38
#define _LALT	0xB8
#define  LCTL	0x1D
#define _LCTL	0x9D
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

static uint8_t shift = 0, alt = 0, ctl = 0;
static uint8_t flush = 0;
//static uint8_t *_buf;
static uint32_t i = 0;
//static uint32_t _len = 0;
//static process_t *p;

void tty_kbd(uint8_t scancode)
{

	tty_device_t *tty = cur_tty->p;

	uint64_t cur_pdpt = 0;
	if(tty->p) cur_pdpt = switch_pdpt(tty->p->pdpt);

	if(scancode ==  LSHIFT || scancode ==  RSHIFT) { shift = 1; goto done; }
	if(scancode == _LSHIFT || scancode == _RSHIFT) { shift = 0; goto done; }
	if(scancode == CAPS) { shift = !shift; goto done; }
	
	if(scancode == LALT)	{ alt   = 1; goto done; }
	if(scancode == _LALT)	{ alt   = 0; goto done; }
	if(scancode == LCTL)	{ ctl   = 1; goto done; }
	if(scancode == _LCTL)	{ ctl   = 0; goto done; }
	
/*	extern void tty_switch(uint32_t);

	// Special keys
	if(alt && scancode == ESC) 
	{
		tty_switch(0);
		goto done;
	}
		
	if(alt && scancode == F1)
	{
		tty_switch(1);
		goto done;
	}
*/	
	uint8_t buf[2];
	buf[1] = '\0';

	if(ctl && kbd_us[scancode] == 'c')
	{
		if(tty->buf) tty->buf[i] = '\0';
		if(tty->p)
		{
			signal_send(tty->p, SIGINT);
			tty->p->stat.rax = -1;
		}	
		i = 0;
		tty->p   = NULL;
		tty->buf = NULL;
		tty->len = 0;
		goto done;
	}

	if(scancode < 60) 
	{
		if(shift) buf[0] = kbd_us_shift[scancode];
		else 
		buf[0] = kbd_us[scancode];
		
		
		if(tty->p)
			(i < tty->len)?(tty->buf[i++] = buf[0]):(tty->buf[i] = '\0');
		
		if(buf[0] == '\b')
		{
			if(tty->buf && i)
			{
				tty->buf[--i] = '\0';
				if(i)
				{
					--i;
					ttydev_write(cur_tty, 0, 1, buf);
				}
			}
		}
		else ttydev_write(cur_tty, 0, 1, buf);
		
		if(kbd_us[scancode] == '\n') 
		{
				if(tty->buf) tty->buf[i] = '\0';
				if(tty->p)
				{
					tty->p->status = READY;
					tty->p->stat.rax = i;
				}
				i = 0;
				tty->p   = NULL;
				tty->buf = NULL;
				tty->len = 0;
		}
	}
	
	done:
	if(cur_pdpt) switch_pdpt(cur_pdpt);
	return;

}

uint32_t ttydev_read(inode_t *inode, uint64_t offset_unused, uint64_t len, void *buf)
{
	tty_device_t *tty = inode->p;
	if(!tty->p) tty->p = current_process;
	tty->p->status = WAITING;
	tty->buf = buf;
	tty->len = len;
}

/*
void ttym_invoke(tty_device_t *tty, uint32_t req, ...)
{
	va_list args;
	va_start(args, req);
	
	switch(req)
	{
		case TTYM_WRITE: //(buf, size, fg, bg)
			{
				uint8_t  *buf	= va_arg(args, uint8_t*);
				uint32_t size	= va_arg(args, uint32_t);
				uint32_t fg		= va_arg(args, uint32_t);
				uint32_t bg		= va_arg(args, uint32_t);
				uint32_t i, cur = 0;
				for(i = 0; i < size; ++i)
				{
					tty->master->console->fs->ioctl(
						tty->master->console,
						0,	// request ( Draw Character )
						(tty->pos + i) % tty->cols, 	//posx
						(tty->pos + i) / tty->cols, 	//posy
						buf[i],
						fg,
						bg
					);
				}
				tty->pos += size;
				
				tty->master->console->fs->ioctl(
					tty->master->console,
					1,	// request ( Set Cursor )
					tty->pos
					);
			}
			break;
		case TTYM_SWITCH:
			
			debug("tty switch");
			break;
		default:
			break;
	}
}
*/

dev_t ttydev = 
	(dev_t)
	{
		.probe = &ttydev_probe,
		.name  = "ttydev",
		.read  = &ttydev_read,
		.write = &ttydev_write,
	};
