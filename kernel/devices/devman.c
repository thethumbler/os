#include <system.h>
#include <device.h>
#include <debug.h>
#include <devfs.h>
#include <tty.h>
#include <ata.h>

dev_t * devices[] = 
{
	&condev,	/* Virtual Console driver */
	&ttydev,	/* TTY driver */
	&atadev,
	NULL,
};

static void devman_init(void)
{
	debug("Loading device drivers ");
	dev_t **dev = devices;
	
	while(*dev)
	{
		(*dev)->probe(&dev_root);
		++dev;
		debug(".");
	}
	
	debug(" [done]\n");
	
	vfs_mount(vfs_root, "/dev", &dev_root);
}

devman_t devman = 
{
	.init = devman_init,
};
