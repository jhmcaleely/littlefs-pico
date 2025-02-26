#ifndef pico_lfs_hal_h
#define pico_lfs_hal_h

#include "../littlefs/lfs.h"

// block device functions required for littlefs
int pico_read_flash_block(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int pico_prog_flash_block(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int pico_erase_flash_block(const struct lfs_config *c, lfs_block_t block);
int pico_sync_flash_block(const struct lfs_config *c);

// Pico's flash can be programed in 256byte pages, and must be erased in 4K pages.
#define PICO_ERASE_PAGE_SIZE 4096
#define PICO_PROG_PAGE_SIZE 256

#endif