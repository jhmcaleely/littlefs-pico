#include <string.h>

#include "pico_lfs_hal.h"

#include <pico/flash.h>
#include <hardware/flash.h>

#include "pico_flash_fs.h"

// We need an offset in bytes for erase and program operations.
#define FLASHFS_FLASH_OFFSET ((const size_t)(FLASHFS_BASE_ADDR - PICO_FLASH_BASE_ADDR))

/*
 * Read from the flash device. Pico's flash is memory mapped, so memcpy will work well
 */
int pico_read_flash_block(const struct lfs_config *c, 
                          lfs_block_t block,
                          lfs_off_t off, 
                          void *buffer, 
                          lfs_size_t size) {

    memcpy(buffer, FLASHFS_BASE_ADDR + block * PICO_ERASE_PAGE_SIZE + off, size);
    return LFS_ERR_OK;
}

/*
 * The Pico SDK provides flash_safe_execute which can be used to wrap write (erase/program)
 * operations to the flash device. The implementation varies, depending on your target 
 * configuration. It will disable interupts, and pause code execution on core 1 if needed.
 * 
 * Using it needs the underlying functions to be passed parameters through a single data
 * pointer, hence the need for a couple of structures to pass them.
 */

struct prog_param {
    uint32_t flash_offs;
    const uint8_t *data; 
    size_t count;
};

static void call_flash_range_program(void* param) {
    struct prog_param* p = (struct prog_param*)param;
    flash_range_program(p->flash_offs, p->data, p->count);
}

/*
 * Write (program) a block of data to the flash device. 
 */
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

/*
 * Erase a block of flash, in preparation for future writes
 */
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

/*
 * The need for this function is unclear. As of writing, it has little 
 * documentation. If we access flash with caching disabled (via XIP_NOCACHE_NOALLOC_BASE)
 * I imagine it may be doing nothing. No provision for wrapping it in flash_safe_execute
 * appears to exist.
 */
int pico_sync_flash_block(const struct lfs_config *c) {
    flash_flush_cache();
    return LFS_ERR_OK;
}