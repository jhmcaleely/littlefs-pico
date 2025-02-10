#ifndef pico_flash_fs_h
#define pico_flash_fs_h

#define PICO_PROG_PAGE_SIZE 256
#define PICO_ERASE_PAGE_SIZE 4096
#define PICO_FLASH_BASE_ADDR  0x10000000
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)

#define FLASHFS_BLOCK_COUNT 128
#define FLASHFS_SIZE_BYTES (PICO_ERASE_PAGE_SIZE * FLASHFS_BLOCK_COUNT)

// A start location counted back from the end of the device.
#define FLASHFS_BASE_ADDR (PICO_FLASH_BASE_ADDR + PICO_FLASH_SIZE_BYTES - FLASHFS_SIZE_BYTES)

#endif