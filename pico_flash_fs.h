#ifndef pico_flash_fs_h
#define pico_flash_fs_h

// Defines one region of flash to use for a filesystem. The size is a multiple of
// the 4096 byte erase size. We calculate it's location working back from the end of the
// flash device, so that code flashed at the start of the device will not collide.
// Pico's have a 2Mb flash device, so we're looking to be less than 2Mb.

// 128 blocks will reserve a 512K filsystem - 1/4 of the 2Mb device on a Pico
#define FLASHFS_BLOCK_COUNT 128
#define FLASHFS_SIZE (PICO_ERASE_PAGE_SIZE * FLASHFS_BLOCK_COUNT)

// We access the flash fs with two addressing modes - a memory address for 
// reading, and an offset (in bytes) for erase/programming. these constants
// are the start of our filesystem in the device.

#define FLASHFS_BASE_ADDR ((const uint8_t*)(PICO_FLASH_BASE_ADDR + PICO_FLASH_SIZE_BYTES - FLASHFS_SIZE))
#define FLASHFS_FLASH_OFFSET ((const size_t)(FLASHFS_BASE_ADDR - PICO_FLASH_BASE_ADDR))


#endif