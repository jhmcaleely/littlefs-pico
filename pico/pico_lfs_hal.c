#include <string.h>

#include "pico_lfs_hal.h"

#include <pico/flash.h>
#include <hardware/flash.h>

#include "../pico_flash_fs.h"

// littlefs hal for Pico. Assumes only one filesystem will be present in the device, unlike
// host-uf2 alongside, which can potentially write multiple filesystems to different areas of
// the flash device.

// We need an offset in bytes for erase and program operations.
#define FLASHFS_FLASH_OFFSET ((const size_t)(FLASHFS_BASE_ADDR - XIP_MAIN_BASE))

uint32_t flash_device_offset(uint32_t block, uint32_t offset) {
    return FLASHFS_FLASH_OFFSET + block * PICO_ERASE_PAGE_SIZE + offset;
}

/*
 * Read from the flash device. Pico's flash is memory mapped, so memcpy will work well.
 *
 * Pico's flash device appears at XIP_MAIN_BASE, and reads are identical to reading from memory.
 * The flash device has a cache, and the device is mapped to 4 different locations in the 
 * address map. Which address range you use, determines the cache behaviour.
 * For littlefs, which has it's own cache in RAM, we use the alias (XIP_NOCACHE_NOALLOC_BASE) 
 * that skips the cache entirely This also means we don't have to do cache maintenance when 
 * we write to the flash device.
 */
int pico_read_flash_block(const struct lfs_config *c, 
                          lfs_block_t block,
                          lfs_off_t off, 
                          void *buffer, 
                          lfs_size_t size) {

    uint32_t offset = flash_device_offset(block, off);
    uint32_t fsAddress = XIP_NOCACHE_NOALLOC_BASE + offset;

    memcpy(buffer, (const uint8_t*) fsAddress, size);
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

    // if using cached memory addresses for read, we would need to 
    // remove cache references to this range here.
}

/*
 * Write (program) a block of data to the flash device. 
 */
int pico_prog_flash_block(const struct lfs_config *c, 
                          lfs_block_t block,
                          lfs_off_t off, 
                          const void *buffer, 
                          lfs_size_t size) {

    struct prog_param p = {
        .data = buffer,
        .count = size
    };
    p.flash_offs = flash_device_offset(block, off);

    int rc = flash_safe_execute(call_flash_range_program, &p, UINT32_MAX);
    if (rc == PICO_OK) {
        return LFS_ERR_OK;
    }
    else {
        return LFS_ERR_IO;
    }
}

// we only need to pass one parameter, so cast it into the pointer.
static void call_flash_range_erase(void* param) {
    uint32_t offset = (uint32_t)param;
    flash_range_erase(offset, PICO_ERASE_PAGE_SIZE);

    // if using cached memory addresses for read, we would need to 
    // remove cache references to this range here.
}

/*
 * Erase a block of flash, in preparation for future writes
 */
int pico_erase_flash_block(const struct lfs_config *c, lfs_block_t block) {
    
    uint32_t offset = flash_device_offset(block, 0);
    
    int rc = flash_safe_execute(call_flash_range_erase, (void*)offset, UINT32_MAX);
    if (rc == PICO_OK) {
        return LFS_ERR_OK;
    }
    else {
        return LFS_ERR_IO;
    }
}

/*
 * Our writes appear to be atomic without the need for a second 'flush' to complete
 * any pending writes.
 */
int pico_sync_flash_block(const struct lfs_config *c) {
    
    return LFS_ERR_OK;
}