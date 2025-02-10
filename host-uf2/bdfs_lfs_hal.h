#ifndef bdfs_lfs_hal_h
#define bdfs_lfs_hal_h

#include "../littlefs/lfs.h"

struct block_device;

void bdfs_create_hal_at(struct lfs_config* c, struct block_device* bd, uint32_t fs_base_address);
void bdfs_destroy_hal(struct lfs_config* c);

// block device functions required for littlefs
int bdfs_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int bdfs_prog_page(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int bdfs_erase_block(const struct lfs_config* c, lfs_block_t block);
int bdfs_sync_block(const struct lfs_config* c);

#endif