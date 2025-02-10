#include <stdio.h>

#include "../littlefs/lfs.h"

#include "block_device.h"
#include "bdfs_lfs_hal.h"
#include "pico_flash_device.h"
#include "../pico_flash_fs.h"


// configuration of the filesystem is provided by this struct
struct lfs_config cfg = {
    // block device operations
    .read  = bdfs_read,
    .prog  = bdfs_prog_page,
    .erase = bdfs_erase_block,
    .sync  = bdfs_sync_block,

    // block device configuration

    .read_size = 1,
    
    .prog_size = PICO_PROG_PAGE_SIZE,
    .block_size = PICO_ERASE_PAGE_SIZE,

    // the number of blocks we use for a flash fs.
    .block_count = FLASHFS_BLOCK_COUNT,

    // cache needs to be a multiple of the programming page size.
    .cache_size = PICO_PROG_PAGE_SIZE * 1,

    .lookahead_size = 16,
    .block_cycles = 500,
};

lfs_t lfs;
lfs_file_t file;

void readuf2(const char * input, struct block_device* bd) {
    FILE* iofile = fopen(input, "rb");
    if (iofile) {
        bdReadFromUF2(bd, iofile);
        fclose(iofile);
    }
}

void writeuf2(const char * input, struct block_device* bd) {
    FILE* iofile = fopen(input, "wb");
    if (iofile) {
        bdWriteToUF2(bd, iofile);
    
        fclose(iofile);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        printf("Usage:\n");
        printf("  %s iofile\n", argv[0]);
        printf("  %s infile outfile\n", argv[0]);
        exit(1);
    }

    const char* infile = argv[1];
    const char* outfile = argc == 3 ? argv[2] : infile;

    struct block_device* bd = bdCreate(PICO_FLASH_BASE_ADDR);
    bdfs_create_hal_at(&cfg, bd, FLASHFS_BASE_ADDR);
    readuf2(infile, bd);

    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // release any resources we were using
    lfs_unmount(&lfs);
    writeuf2(outfile, bd);

    bdfs_destroy_hal(&cfg);
    bdDestroy(bd);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
}
