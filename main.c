#include <stdio.h>
#include <pico/stdlib.h>

#include "littlefs/lfs.h"

// configuration of the filesystem is provided by this struct
struct lfs_config cfg = {
    // block device operations
    .read  = NULL,
    .prog  = NULL,
    .erase = NULL,
    .sync  = NULL,

    // block device configuration
    .read_size = 1,
    .prog_size = 0,
    .block_size = 0,
    .block_count = 0,
    .cache_size = 0 * 0,
    .lookahead_size = 16,
    .block_cycles = 500,
};


lfs_t lfs;
lfs_file_t file;

int main()
{
    stdio_init_all();

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

    // print the boot count
    printf("boot_count: %d\n", boot_count);
}
