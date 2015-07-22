#ifndef _EXT2_H
#define _EXT2_H

#include <system.h>
#include <vfs.h>

#define	__packed	__attribute__((packed))

typedef struct
{
	uint8_t preallocate_for_directories : 1;
	uint8_t afs_server_inodes : 1;
	uint8_t has_journaling : 1;		// for EXT 3
	uint8_t inodes_have_extended_attributes : 1;
	uint8_t can_be_resized : 1;
	uint8_t directories_use_hash_index : 1;
} __packed optional_features_flags_t;

typedef struct
{
	uint8_t compressed : 1;
	uint8_t directories_has_types : 1;
	uint8_t needs_to_replay_journal : 1;
	uint8_t uses_journal_device : 1;
} __packed required_features_flags_t;

typedef struct
{
	uint8_t sparse_sb_and_gdt : 1;
	uint8_t uses_64bit_file_size : 1;
	uint8_t directories_contents_are_binary_tree : 1;
} __packed read_only_features_flags_t;

typedef struct
{
	uint32_t	inodes_count;
	uint32_t	blocks_count;
	uint32_t	su_blocks_count;
	uint32_t	unallocated_blocks_count;
	uint32_t	unallocated_inodes_count;
	uint32_t	su_block_number;
	uint32_t	block_size;
	uint32_t	fragment_size;
	uint32_t	blocks_per_block_group;
	uint32_t	fragments_per_block_group;
	uint32_t	inodes_per_block_group;
	uint32_t	last_mount_time;
	uint32_t	last_written_time;
	uint16_t	times_mounted;	// after last fsck
	uint16_t	allowed_times_mounted;
	uint16_t	ext2_signature;
	uint16_t	filesystem_state;
	uint16_t	on_error;
	uint16_t	minor_version;
	uint32_t	last_fsck;
	uint32_t	time_between_fsck;	// Time allowed between fsck(s)
	uint32_t	os_id;
	uint32_t	major_version;
	uint16_t	uid;
	uint16_t	gid;
	
	// For extended superblock fields ... which is always assumed
	uint32_t	first_non_reserved_inode;
	uint16_t	inode_size;
	uint16_t	sb_block_group;
	optional_features_flags_t	optional_features;
	required_features_flags_t	required_features;
	read_only_features_flags_t	read_only_features;
	uint8_t		fsid[16];
	uint8_t		name[16];
	uint8_t		path_last_mounted_to[64];
	uint32_t	compression_algorithms;
	uint8_t		blocks_to_preallocate_for_files;
	uint8_t		blocks_to_preallocate_for_directories;
	uint16_t	__unused__;
	uint8_t		jid[16];	// Journal ID
	uint32_t	jinode;		// Journal inode
	uint32_t	jdevice;	// Journal device
	uint32_t	head_of_orphan_inode_list;	// What the hell is this !?
} __packed ext2_superblock_t;

typedef struct
{
	uint32_t	block_usage_bitmap;
	uint32_t	inode_usage_bitmap;
	uint32_t	inode_table;
	uint16_t	unallocated_blocks_count;
	uint16_t	unallocated_inodes_count;
	uint16_t	directories_count;
	uint8_t		__unused__[14];
} __packed block_group_descriptor_t;

#define ext2_inode_type_FIFO	0x1	// FIFO pipe
#define ext2_inode_type_CHR		0x2	// Character device
#define ext2_inode_type_DIR		0x4	// Directory
#define ext2_inode_type_BLK		0x6	// Block device
#define ext2_inode_type_RGL		0x8	// Regular file
#define ext2_inode_type_SLINK	0xA	// Symbolic link
#define ext2_inode_type_SCKT	0xC	// Unix socket

typedef struct
{
	uint8_t other_exec	: 1;
	uint8_t	other_write : 1;
	uint8_t	other_read  : 1;
	uint8_t	group_exec  : 1;
	uint8_t	group_write : 1;
	uint8_t	group_read  : 1;
	uint8_t	user_exec   : 1;
	uint8_t	user_write  : 1;
	uint8_t	user_read   : 1;
} __packed ext2_inode_permissions_t;

typedef struct
{
	uint16_t	permissions : 12;
	uint16_t	type : 4;
	uint16_t	uid;
	uint32_t	size;
	uint32_t	last_access_time;
	uint32_t	creation_time;
	uint32_t	last_modified_time;
	uint32_t	deletion_time;	// Wha !?
	uint16_t	gid;
	uint16_t	hard_links_count;
	uint32_t	sectors_count;
	uint32_t	flags;
	uint32_t	os_specific1;
	uint32_t	direct_pointer[12];
	uint32_t	singly_indirect_pointer;
	uint32_t	doubly_indirect_pointer;
	uint32_t	triply_indirect_pointer;
	uint32_t	generation_number;
	uint32_t	extended_attribute_block;
	uint32_t	size_high;
	uint32_t	fragment;
	uint8_t		os_specific2[12];
} __packed ext2_inode_t;

typedef struct
{
	uint32_t	inode;
	uint16_t	size;
	uint8_t		name_length;
	uint8_t		type;
	uint8_t		name[0];	// Name will be here
} __packed ext2_dentry_t;

#define ext2_dir_type_BAD	0
#define ext2_dir_type_RGL	1
#define ext2_dir_type_DIR	2
#define ext2_dir_type_CHR	3
#define ext2_dir_type_BLK	4
#define ext2_dir_type_FIFO	5
#define ext2_dir_type_SCKT	6
#define ext2_dir_type_SLINK	7

extern fs_t ext2fs;

#endif
