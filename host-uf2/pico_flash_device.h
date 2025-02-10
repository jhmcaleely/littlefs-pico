#ifndef pico_flash_device_h
#define pico_flash_device_h

// A few constants also found in the Pico C SDK so that the host code does not depend on the Pico SDK.

#define PICO_PROG_PAGE_SIZE 256
#define PICO_ERASE_PAGE_SIZE 4096
#define PICO_FLASH_BASE_ADDR  0x10000000
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)

#endif