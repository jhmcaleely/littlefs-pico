#ifndef block_device_h
#define block_device_h

#include <stdint.h>
#include <stdbool.h>

// A few constants also found in the Pico C SDK so that the host code does not depend on the Pico SDK.

#define PICO_PROG_PAGE_SIZE 256
#define PICO_ERASE_PAGE_SIZE 4096
#define PICO_FLASH_BASE_ADDR  0x10000000
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)

struct block_device;

struct block_device* bdCreate(uint32_t flash_base_address);
void bdDestroy(struct block_device* bd);

void bdDebugPrint(struct block_device* bd);

void bdEraseBlock(struct block_device* bd, uint32_t address);
void bdWrite(struct block_device* bd, uint32_t address, const uint8_t* data, size_t size);
void bdRead(struct block_device* bd, uint32_t address, uint8_t* buffer, size_t size);

void bdReadFromUF2(struct block_device* bd, FILE* input);
void bdWriteToUF2(struct block_device* bd, FILE* output);


#endif