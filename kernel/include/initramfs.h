#ifndef _INITRAMFS_H
#define _INITRAMFS_H

#include <system.h>
#include <vfs.h>

extern fs_t initramfs;

typedef struct
{
  uint16_t magic;
  uint16_t dev;
  uint16_t ino;
  uint16_t mode;
  uint16_t uid;
  uint16_t gid;
  uint16_t nlink;
  uint16_t rdev;
  uint16_t mtimes[2];
  uint16_t namesize;
  uint16_t filesize[2];
} cpio_hdr_t;

#endif
