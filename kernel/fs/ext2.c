#include <system.h>
#include <vfs.h>
#include <ext2.h>
#include <debug.h>
#include <device.h>
#include <string.h>

typedef struct
{
	ext2_superblock_t *sb;
	uint32_t inode;
}ext2_private_t;

uint32_t *get_block(dev_t *dev, ext2_superblock_t *sb, uint32_t number, void *buf)
{
	uint32_t bs = 1024 << sb->block_size;
	//uint32_t *ret = kmalloc(bs);
	dev->read(dev, number * bs, bs, buf);
	return buf;
}

uint32_t *get_block_ptrs(dev_t *dev, ext2_superblock_t *sb, ext2_inode_t *inode)
{
	uint32_t bs = 1024 << sb->block_size;
	
	uint32_t isize = inode->size / bs + (inode->size%bs?1:0);
	uint32_t *blks = kmalloc(sizeof(uint32_t) * (isize + 1));

	// The size is always saved as the first entry of the returned array
	blks[0] = isize;
	
	uint32_t k;
	for(k = 1; k < 12 && k <= isize; ++k)
		blks[k] = inode->direct_pointer[k - 1];
	
	if(isize <= 12)	// Was that enough !?
		return blks;

	isize -= 12;
	
	uint32_t *tmp = get_block(dev, sb, inode->singly_indirect_pointer, kmalloc(bs));
	
	
	for(k = 0; k < (bs/4) && k <= isize; ++k)
		blks[13 + k] = tmp[k];
		
	kfree(tmp);
	
	if(isize <= (bs/4))	// Are we done yet !?
		return blks;
		
	// TODO : Support doubly & triply indirect pointers
	debug("File size too big\n");
	for(;;);
}

inode_t *ext2_load(dev_t *dev)
{
	ext2_superblock_t *sb = kmalloc(sizeof(*sb));
	dev->read(dev, 1024, sizeof(*sb), sb);
	//uint32_t *b = get_block(dev, sb, 2);
	
	uint32_t bs = 1024 << sb->block_size;
	uint32_t is = sb->inode_size;
	
	block_group_descriptor_t *bgd = kmalloc(sizeof(*bgd));
	dev->read(dev,  2 * bs, sizeof(*bgd), bgd);
	
	ext2_inode_t *root_inode = kmalloc(is);
	dev->read(dev, bs * bgd->inode_table + is, is, root_inode);
	
	uint32_t *ptrs = get_block_ptrs(dev, sb, root_inode);
	uint32_t count = *ptrs++;
	
	uint32_t k;
		
	ext2_dentry_t *_d = kmalloc(bs), *d;
	
	inode_t *ext2_vfs = kmalloc(sizeof(inode_t));
	ext2_vfs->name = strdup("hd");
	ext2_vfs->type = FS_DIR;
	ext2_vfs->list = kmalloc(sizeof(dentry_t));
	ext2_vfs->list->count = 0;
	ext2_vfs->list->head = NULL;
	inode_t *_tmp = NULL;
	
	for(k = 0; k < count; ++k)
	{
		d = get_block(dev, sb, *(ptrs + k), _d);
		if(d)
		{
			uint32_t size = 0;
			while(size <= bs)
			{
				if(!d->size || size + d->size > 1024) break;
				//if(d->name_length)
				{
					//uint8_t *s = strndup(d->name, d->name_length);
					//debug("%s\n", s);
					//kfree(s);
				}
				
				if(!_tmp)
					ext2_vfs->list->head = _tmp = kmalloc(sizeof(inode_t));
				else
					_tmp = _tmp->next = kmalloc(sizeof(inode_t));
				
				_tmp->next = NULL;
				_tmp->name = strndup(d->name, d->name_length);
				_tmp->size = d->size;
				_tmp->type = FS_FILE;
				_tmp->dev  = dev;
				_tmp->fs   = &ext2;
				_tmp->p = kmalloc(sizeof(ext2_private_t));
				*(ext2_private_t*)_tmp->p = (ext2_private_t)
						{
							.sb = sb,
							.inode = d->inode,
						};
				ext2_vfs->list->count++;
				
				size += d->size;		
				d = (uint64_t)d + d->size;
			}
			//kfree(d);
		}
	}
	kfree(_d);


	// Mounting it
	vfs_create(vfs_root, "/", ext2_vfs);
}

void ext2_read(inode_t *inode, uint8_t *buf, uint32_t size)
{
	debug("Reading file %s\n", inode->name);
	ext2_private_t *p = inode->p;
	uint32_t bs = 1024 << p->sb->block_size;
	uint32_t block_group = (p->inode - 1) / p->sb->inodes_per_block_group;
	block_group_descriptor_t *bgd = kmalloc(sizeof(*bgd));
	inode->dev->read(inode->dev, 2 * bs + block_group, sizeof(*bgd), bgd);
	uint32_t index = (p->inode - 1) % p->sb->inodes_per_block_group;
	ext2_inode_t *i = kmalloc(*i);
	inode->dev->read(inode->dev, 
					bgd->inode_table * bs + index * p->sb->inode_size,
					sizeof(*i),
					i);
					
	uint32_t *ptrs = get_block_ptrs(inode->dev, p->sb, i);
	debug("ptrs %lx\n", ptrs);

	uint32_t count = *ptrs++;

	while(count > 1 && size > count * bs)
	{
		debug("loop\n");
		inode->dev->read(inode->dev, *ptrs++ * bs, bs, buf);
		buf += bs;
		--count; 
	}

	// Reading the last block and truncating
	uint32_t trunc_size = MIN(inode->size % bs, size % bs);
	inode->dev->read(inode->dev, *ptrs * bs, trunc_size, buf);
}

file_t *ext2_open(inode_t *inode)
{
	file_t *ret = kmalloc(sizeof(file_t));
	ret->pos = 0;
	ret->size = inode->size;
	ret->buf = kmalloc(30);
	debug("size %d\n", inode->size);
	inode->fs->read(inode, ret->buf, 20);
	return ret;
}

fs_t ext2 = 
	(fs_t)
	{
		//.name = strdup("ext2"),
		.load = ext2_load,
		.open = ext2_open,
		.read = ext2_read,
		.write = NULL,
	};
