# host-uf2

A C/stdlib tool that combines littlefs and uf2. It implements the littlefs sample such that you can run it on your host PC, and then flash the resulting uf2 to a pico, with the filesystem preserved. Repeated runs of host-uf2 will increase the 'boot count'.

Can be used with a standard pico firmware image as input, in which case the firmware image will be combined with the filesystem into one uf2 that will have both within the same uf2 image.


block_device.c, .h: a RAM byte array that tracks writes to simulate the block/page write behaviour of the Pico flash.

bdfs_lfs_hal.c, .h: A HAL for littlefs that exposes the RAM block_device.

main.c: the sample code from littlefs, lightly modified to read and write uf2 files
