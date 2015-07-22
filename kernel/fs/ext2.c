#include <system.h>
#include <vfs.h>
#include <ext2.h>
#include <debug.h>
#include <device.h>
#include <string.h>
#include <kmem.h>

typedef struct
{
	ext2_superblock_t *sb;
	uint32_t inode;
}ext2_private_t;


ext2_inode_t *ext2_get_inode(inode_t *dev_inode, ext2_superblock_t *sb, uint32_t inode)
{
	uint32_t bs = 1024 << sb->block_size;
	uint32_t block_group = (inode - 1) / sb->inodes_per_block_group;
	block_group_descriptor_t *bgd = kmalloc(sizeof(*bgd));
	dev_inode->fs->read(dev_inode, 2 * bs + block_group, sizeof(*bgd), bgd);
	uint32_t index = (inode - 1) % sb->inodes_per_block_group;
	ext2_inode_t *i = kmalloc(sizeof(*i));
	dev_inode->fs->read(dev_inode, bgd->inode_table * bs + index * sb->inode_size, sizeof(*i), i);
	return i;
}

uint32_t *get_block(inode_t *dev_inode, ext2_superblock_t *sb, uint32_t number, void *buf)
{
	uint32_t bs = 1024 << sb->block_size;
	//uint32_t *ret = kmalloc(bs);
	dev_inode->fs->read(dev_inode, number * bs, bs, buf);
	return buf;
}

uint32_t *get_block_ptrs(inode_t *dev_inode, ext2_superblock_t *sb, ext2_inode_t *inode)
{
	uint32_t bs = 1024 << sb->block_size;
	
	uint32_t isize = inode->size / bs + (inode->size % bs ? 1 : 0 );
	uint32_t *blks = kmalloc(sizeof(uint32_t) * (isize + 1));

	// The size is always saved as the first entry of the returned array
	blks[0] = isize;
	
	uint32_t k;
	for(k = 1; k <= 12 && k <= isize; ++k)
		blks[k] = inode->direct_pointer[k - 1];
	
	if(isize <= 12)	// Was that enough !?
		return blks;

	isize -= 12;
	

	uint32_t *tmp = get_block(dev_inode, sb, inode->singly_indirect_pointer, kmalloc(bs));
	
	for(k = 0; k < (bs/4) && k <= isize; ++k)
		blks[13 + k] = tmp[k];
		
	kfree(tmp);
	
	if(isize <= (bs/4))	// Are we done yet !?
		return blks;
	
	isize -= (bs/4);
	
	tmp = get_block(dev_inode, sb, inode->doubly_indirect_pointer, kmalloc(bs));
	uint32_t *tmp2 = kmalloc(bs);
	
	uint32_t j;
	for(k = 0; k < (bs/4) && (k * (bs/4)) <= isize; ++k)
	{
		tmp2 = get_block(dev_inode, sb, tmp[k], tmp2);
		for(j = 0; j < (bs/4) && (k * (bs/4) + j) <= isize; ++j)
		{
			blks[13 + bs/4 + k * bs/4 + j] = tmp2[j];
		}
	}

	if(isize <= (bs/4)*(bs/4))
		return blks;

	kfree(tmp);
	
	// TODO : Support triply indirect pointers
	debug("File size too big\n");
	for(;;);
}

inode_t *ext2_load(inode_t *dev_inode)
{
	ext2_superblock_t *sb = kmalloc(sizeof(*sb));
	vfs_read(dev_inode, 1024, sizeof(*sb), sb);
	
	if(sb->ext2_signature != 0xEF53) return NULL;
	//uint32_t *b = get_block(dev, sb, 2);
	
	uint32_t bs = 1024 << sb->block_size;	// Block size
	uint32_t is = sb->inode_size;			// Inode size
	
	block_group_descriptor_t *bgd = kmalloc(sizeof(*bgd));
	vfs_read(dev_inode, (sb->block_size ? 1 : 2) * bs, sizeof(*bgd), (void*)bgd);
	
	ext2_inode_t *root_inode = kmalloc(is);
	vfs_read(dev_inode, bs * bgd->inode_table + is, is, root_inode);
	
	uint32_t *ptrs = get_block_ptrs(dev_inode, sb, root_inode);
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
		d = (ext2_dentry_t*)(get_block(dev_inode, sb, *(ptrs + k), _d));
		if(d)
		{
			uint32_t size = 0;
			while(size <= bs)
			{
				if((!d->size) || (size + d->size > bs)) break;
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
				debug("%s\n", _tmp->name);
				ext2_inode_t *_inode = ext2_get_inode(dev_inode, sb, d->inode);
				_tmp->size = _inode->size;
				kfree(_inode);
				_tmp->type = FS_FILE;
				_tmp->dev  = dev_inode->dev;
				_tmp->fs   = &ext2fs;
				_tmp->p = kmalloc(sizeof(ext2_private_t));
				*(ext2_private_t*)_tmp->p = (ext2_private_t)
						{
							.sb = sb,
							.inode = d->inode,
						};
				ext2_vfs->list->count++;
				
				size += d->size;		
				d = (ext2_dentry_t*)((uint64_t)d + d->size);
			}
			//kfree(d);
		}
	}
	
	kfree(_d);

	return ext2_vfs;
}

uint64_t ext2_read(inode_t *inode, uint64_t offset, uint64_t size, void *buf)
{
	uint32_t _size = size;
	
	debug("Reading file %s\n", inode->name);
	ext2_private_t *p = inode->p;
	
	uint32_t bs = 1024 << p->sb->block_size;
	ext2_inode_t *i = ext2_get_inode(inode, p->sb, p->inode);
					
	uint32_t *ptrs = get_block_ptrs(inode, p->sb, i);
	uint32_t count = *ptrs++;

	uint32_t skip_blocks = offset/bs; // Blocks to be skipped
	uint32_t skip_bytes  = offset%bs; // Bytes  to be skipped
	
	if(count < skip_blocks) return 0;
	
	count -= skip_blocks;
	ptrs  += skip_blocks;
	
	// Now let's read the first part ( size <= bs )
	uint32_t first_part_size = (count > 1)?MIN(bs - skip_bytes, size):MIN(size, inode->size);
	inode->dev->read(inode, (*ptrs++) * bs + skip_bytes, first_part_size, buf);
	buf += first_part_size;
	size -= first_part_size;
	--count;
	
	// Now let's read the second part ( size is a multible of bs )
	while(count > 1 && size > bs)
	{
		inode->dev->read(inode, *ptrs++ * bs, bs, buf);
		buf += bs;
		size -= bs;
		--count;
	}
	
	// Now let's read the last part ( size <= bs)
	if(count)
	{
		uint32_t last_part_size = MIN(size, inode->size - ( _size - size ));
		inode->dev->read(inode, (*ptrs++) * bs, last_part_size, buf);
		buf += last_part_size;
		size -= last_part_size;
		--count;
	}
	
	return _size - size;

}

static uint32_t ext2_mount(inode_t *dst, inode_t *src)
{
	inode_t *i = ext2_load(src);

	if(!i) return -1;
	vfs_mount(dst, "/", i);

	return 1;
}

/*
file_t *ext2_open(inode_t *inode)
{
	file_t *ret = kmalloc(sizeof(file_t));
	ret->pos = 0;
	ret->size = inode->size;
	ret->buf = kmalloc(30);
	debug("size %d\n", inode->size);
	inode->fs->fs->read(inode, 0ret->buf, 20);
	return ret;
}
*/
fs_t ext2fs = 
	(fs_t)
	{
		.name = "ext2",
		.load = ext2_load,
		//.open = NULL,
		.read = ext2_read,
		.write = NULL,
		.mount = &ext2_mount,
	};
