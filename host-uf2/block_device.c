#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "block_device.h"

#include "pico_flash_device.h"
#include "../pico_flash_fs.h"
#include "../uf2/uf2.h"

/*
 * A RAM block device that mimics a Pico Flash device. We can write this 
 * out to a uf2 file for flashing.
 */

#define PICO_UF2_FAMILYID 0xe48bff56
#define PICO_DEVICE_BLOCK_COUNT (PICO_FLASH_SIZE_BYTES / PICO_ERASE_PAGE_SIZE)
#define PICO_FLASH_PAGE_PER_BLOCK (PICO_ERASE_PAGE_SIZE / PICO_PROG_PAGE_SIZE)

struct block_device {
    uint8_t storage[PICO_FLASH_SIZE_BYTES];

    bool page_present[PICO_DEVICE_BLOCK_COUNT][PICO_FLASH_PAGE_PER_BLOCK];

    uint32_t base_address;
};

struct page_address {
    uint32_t block;
    uint32_t page;
    uint32_t offset;
};

uint32_t bdStorageOffset(uint32_t block, uint32_t page) {
    return block * PICO_ERASE_PAGE_SIZE + page * PICO_PROG_PAGE_SIZE;
}

uint32_t getDeviceBlockNo(struct block_device* bd, uint32_t address) {
    return (address - bd->base_address) / PICO_ERASE_PAGE_SIZE;
}

void bdPageAddresss(struct block_device* bd, struct page_address* ad, uint32_t address) {
    uint32_t page_offset = (address - bd->base_address) % PICO_ERASE_PAGE_SIZE;    
    ad->page = page_offset / PICO_PROG_PAGE_SIZE;
    ad->offset = page_offset % PICO_PROG_PAGE_SIZE;
    ad->block = getDeviceBlockNo(bd, address);
}

struct block_device* bdCreate(uint32_t flash_base_address) {

    struct block_device* bd = malloc(sizeof(struct block_device));

    bd->base_address = flash_base_address;

    for (int b = 0; b < PICO_DEVICE_BLOCK_COUNT; b++) {
        for (int p = 0; p < PICO_FLASH_PAGE_PER_BLOCK; p++) {
            bd->page_present[b][p] = false;
        }
    }

    return bd;
}

void bdDestroy(struct block_device* bd) {
    free(bd);
}

void _bdEraseBlock(struct block_device* bd, uint32_t block) {

    for (int p = 0; p < PICO_FLASH_PAGE_PER_BLOCK; p++) {
        bd->page_present[block][p] = false;
    }
}

void bdEraseBlock(struct block_device* bd, uint32_t address) {

    uint32_t block = getDeviceBlockNo(bd, address);

    _bdEraseBlock(bd, block);
}

void bdDebugPrint(struct block_device* bd) {
    for (int b = 0; b < PICO_DEVICE_BLOCK_COUNT; b++) {
        for (int p = 0; p < PICO_FLASH_PAGE_PER_BLOCK; p++) {
            if (bd->page_present[b][p]) {
                printf("Page [%d, %d]: %08x\n", b, p, bd->base_address + bdStorageOffset(b, p));
            }
        }
    }
}

void _bdWrite(struct block_device* bd, uint32_t block, uint32_t page, const uint8_t* data, size_t size) {

    bd->page_present[block][page] = true;

    uint8_t* target = &bd->storage[bdStorageOffset(block, page)];

    memcpy(target, data, size);
}

void bdWrite(struct block_device* bd, uint32_t address, const uint8_t* data, size_t size) {
    assert(size <= PICO_PROG_PAGE_SIZE);

    struct page_address ad;
    bdPageAddresss(bd, &ad, address);

    assert(ad.offset == 0);

    _bdWrite(bd, ad.block, ad.page, data, size);
}

void _bdRead(struct block_device* bd, uint32_t block, uint32_t page, uint32_t off, uint8_t* buffer, size_t size) {

    uint32_t storage_offset = bdStorageOffset(block, page) + off;

    if (bd->page_present[block][page]) {
        printf("Read   available page");
        memcpy(buffer, &bd->storage[storage_offset], size);
    }
    else {
        printf("Read unavailable page");
    }
    printf("[%d][%d] off %d (size: %lu)\n", block, page, off, size);
}

void bdRead(struct block_device* bd, uint32_t address, uint8_t* buffer, size_t size) {

    struct page_address ad;
    bdPageAddresss(bd, &ad, address);

    _bdRead(bd, ad.block, ad.page, ad.offset, buffer, size);
}

int countPages(struct block_device* bd) {
    int count = 0;

    for (int b = 0; b < PICO_DEVICE_BLOCK_COUNT; b++) {
        for (int p = 0; p < PICO_FLASH_PAGE_PER_BLOCK; p++) {
            if (bd->page_present[b][p]) {
                count++;
            }
        }
    }

    return count;
}

void bdWriteToUF2(struct block_device* bd, FILE* output) {
    int pageTotal = countPages(bd);
    int pageCursor = 0;

    for (int b = 0; b < PICO_DEVICE_BLOCK_COUNT; b++) {
        for (int p = 0; p < PICO_FLASH_PAGE_PER_BLOCK; p++) {
            if (bd->page_present[b][p]) {
                UF2_Block ub;
                ub.magicStart0 = UF2_MAGIC_START0;
                ub.magicStart1 = UF2_MAGIC_START1;
                ub.flags = UF2_FLAG_FAMILY_ID;
                ub.targetAddr = bd->base_address + bdStorageOffset(b, p);
                ub.payloadSize = PICO_PROG_PAGE_SIZE;
                ub.blockNo = pageCursor;
                ub.numBlocks = pageTotal;
                
                // documented as FamilyID, Filesize or 0.
                ub.reserved = PICO_UF2_FAMILYID;

                bdRead(bd, ub.targetAddr, ub.data, PICO_PROG_PAGE_SIZE);

                // Zero fill the undefined space
                memset(&ub.data[PICO_PROG_PAGE_SIZE], 0, sizeof(ub.data) - PICO_PROG_PAGE_SIZE);

                ub.magicEnd = UF2_MAGIC_END;
                
                printf("uf2page: %08x, %d\n", ub.targetAddr, ub.payloadSize);

                fwrite(&ub, sizeof(ub), 1, output);

                pageCursor++;
            }
        }
    }
}

void bdReadFromUF2(struct block_device* bd, FILE* input) {

    UF2_Block ub;
    while (fread(&ub, sizeof(UF2_Block), 1, input)) {
        assert(ub.magicStart0 == UF2_MAGIC_START0);
        assert(ub.magicStart1 == UF2_MAGIC_START1);
        assert(ub.magicEnd == UF2_MAGIC_END);
        // erase a block before writing any pages to it.
        if (((ub.targetAddr - bd->base_address) % PICO_ERASE_PAGE_SIZE) == 0) {
            bdEraseBlock(bd, ub.targetAddr);
        }

        bdWrite(bd, ub.targetAddr, ub.data, ub.payloadSize);
    }
}
