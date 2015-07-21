#include <system.h>
#include <va_list.h>
#include <device.h>
#include <kmem.h>
#include <va_list.h>
#include <tty.h>

static uint32_t console_ioctl(inode_t *inode_unused, uint64_t request, va_list args)
{
	switch(request)
	{
		case 0:	// Draw Character (x, y, chr, fg, bg)
			{
				uint32_t posx	= va_arg(args, uint32_t);
				uint32_t posy	= va_arg(args, uint32_t);
				uint32_t chr	= va_arg(args, uint32_t);
				uint32_t fg		= va_arg(args, uint32_t);
				uint32_t bg		= va_arg(args, uint32_t);
				uint32_t pos = 2 * (posx + posy * 80);
				//debug("Drawing at %d %d\n", posx, posy);
				*(uint8_t*)((uint64_t)&VMA + 0xB8000 + pos++) = chr & 0xFF;
				*(uint8_t*)((uint64_t)&VMA + 0xB8000 + pos++) = fg & 0xF | ((bg << 4) & 0xF0);
			}
			break;
		case 1:	// Set cursor (pos)	NOTE: pos = x + y * tty->cols
			{
				uint32_t pos	= va_arg(args, uint32_t);
				//debug("Set Cur %d\n", pos);
				outb(0x3D4, 0x0F);
		    	outb(0x3D5, (uint8_t)(pos & 0xFF));

	    		outb(0x3D4, 0x0E);
    			outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
			}
			break;
	}
}

static void virtcon_draw(virtcon_device_t *virtcon)
{
	//XXX temp draw
	uint8_t *vmem = (uint8_t*)((uint64_t)&VMA + 0xB8000);
	uint32_t i;
	for(i = 0; i < 80*25; ++i)
	{
		vmem[2 * i + 0] = virtcon->buf[3*i];
		vmem[2 * i + 1] = 0xFF & (virtcon->buf[3*i+1] | virtcon->buf[3*i+2] << 4);
	}
}

static void 
virtcon_putc(virtcon_device_t *virtcon, uint8_t chr, 
			 uint32_t x, uint32_t y, uint32_t fg, uint32_t bg)
{
	if(x > virtcon->w || y > virtcon->h) return;
	uint32_t pos = 3 * (y * virtcon->w + x);
	virtcon->buf[  pos] = chr;
	virtcon->buf[++pos] = fg;
	virtcon->buf[++pos] = bg;
}

static void
virtcon_puts(virtcon_device_t *virtcon, uint8_t *str, uint32_t size,
			 uint32_t x, uint32_t y, uint32_t fg, uint32_t bg)
{
	if(!size) return;
	uint32_t pos = 3 * (y * virtcon->w + x) - 1;
	uint32_t i;
	for(i = 0; i < size; ++i)
	{
		virtcon->buf[++pos] = str[i];
		virtcon->buf[++pos] = fg;
		virtcon->buf[++pos] = bg;
		//virtcon_putc(virtcon, str[i], i%virtcon->w, i/virtcon->w, fg, bg);
	}
}

static void
virtcon_setcur(virtcon_device_t *virtcon, uint32_t c, uint32_t r)
{
	if(c > virtcon->w || r > virtcon->h) return;
	
	uint32_t pos = r * virtcon->w + c;
	if(!virtcon->buf[3 * pos + 1])
		virtcon->buf[3 * pos + 1] = 0x7;

	outb(0x3D4, 0x0F);
   	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void virtcon_scroll(virtcon_device_t *virtcon, uint32_t n)
{
	uint32_t to_copy = (virtcon->h - n) * virtcon->w * 3;
	memcpy(virtcon->buf, virtcon->buf + 3 * n * virtcon->w, to_copy);

	memset(virtcon->buf + to_copy, 0, 3 * n * virtcon->w);
}

virtcon_device_t *get_virtcon(tty_device_t *tty, uint32_t w, uint32_t h)
{
	virtcon_device_t *virtcon = kmalloc(sizeof(virtcon_device_t));
	virtcon->buf  = kmalloc(h * w * 3);	// chr | fg | bg
	memset(virtcon->buf, 0, h * w * 3);
	virtcon->putc = &virtcon_putc;
	virtcon->puts = &virtcon_puts;
	virtcon->setcur = &virtcon_setcur;
	virtcon->scroll = &virtcon_scroll;
	virtcon->draw = &virtcon_draw;
	virtcon->w = w;
	virtcon->h = h;
	return virtcon;
}

dev_t condev = 
	(dev_t)
	{
		.read  = NULL,
		.write = NULL,
		.ioctl = console_ioctl,
	};
