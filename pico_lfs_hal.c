#include <string.h>

#include "pico_lfs_hal.h"

#include <pico/flash.h>
#include <hardware/flash.h>

#include "pico_flash_fs.h"

// We need an offset in bytes for erase and program operations.
#define FLASHFS_FLASH_OFFSET ((const size_t)(FLASHFS_BASE_ADDR - PICO_FLASH_BASE_ADDR))

int pico_read_flash_block(const struct lfs_config *c, 
                          lfs_block_t block,
                          lfs_off_t off, 
                          void *buffer, 
                          lfs_size_t size) {

    memcpy(buffer, FLASHFS_BASE_ADDR + block * PICO_ERASE_PAGE_SIZE + off, size);
    return LFS_ERR_OK;
}

struct prog_param {
    uint32_t flash_offs;
    const uint8_t *data; 
    size_t count;
};

static void call_flash_range_program(void* param) {
    struct prog_param* p = (struct prog_param*)param;
    flash_range_program(p->flash_offs, p->data, p->count);
}

int pico_prog_flash_block(const struct lfs_config *c, 
                          lfs_block_t block,
                          lfs_off_t off, 
                          const void *buffer, 
                          lfs_size_t size) {

    struct prog_param p;
    p.flash_offs = FLASHFS_FLASH_OFFSET + block * PICO_ERASE_PAGE_SIZE + off;
    p.data = buffer;
    p.count = size;

    int rc = flash_safe_execute(call_flash_range_program, &p, UINT32_MAX);
    if (rc == PICO_OK) {
        return LFS_ERR_OK;
    }
    else {
        return LFS_ERR_IO;
    }
}

struct erase_param {
    uint32_t offset;
    size_t size;
};

static void call_flash_range_erase(void* param) {
    struct erase_param* p = (struct erase_param *)param;
    flash_range_erase(p->offset, p->size);
}

int pico_erase_flash_block(const struct lfs_config *c, lfs_block_t block) {
    
    struct erase_param p;
    p.offset = FLASHFS_FLASH_OFFSET + block * PICO_ERASE_PAGE_SIZE;
    p.size = c->block_size;

    int rc = flash_safe_execute(call_flash_range_erase, &p, UINT32_MAX);
    if (rc == PICO_OK) {
        return LFS_ERR_OK;
    }
    else {
        return LFS_ERR_IO;
    }
}

int pico_sync_flash_block(const struct lfs_config *c) {
    flash_flush_cache();
    return LFS_ERR_OK;
}