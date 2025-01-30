# littlefs-pico

A repo to build the example code found in [littlefs/README.md](https://github.com/littlefs-project/littlefs/blob/0494ce7169f06a734a7bd7585f49a9fa91fa7318/README.md) for the Raspberry Pi Pico, using the vscode plugin and Pico SDK.

After installing the Raspberry Pi Pico SDK extensions to VSCode, opening this folder should present build and run options.

Alternatively, if you have the Pico SDK installed:

```
% cmake .
% cmake --build .
```

will create `littlefs-pico.uf2` that can be flashed onto a pico as usual. By default, serial output (from printf, etc) will be available over the USB and uart ports on the pico.

## main.c

The example code that formats a littlefs drive, and counts reboots in a file.

## pico_lfs_hal.c and pico_lfs_hal.h

An implementation of the littelfs block device that uses the Pico C SDK (tested on 2.1) to read/write/erase blocks to the built in flash on a Pico.

## pico_flash_fs.h

A definition of a single flash area to use as a filesystem. The range is at the opposite end of flash to that used by code, so that it is easy to reflash code without clobbering the flash drive.

## Tools

I found '[flash_nuke.utf2](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html#resetting-flash-memory)' useful to reset the pico's flash device for testing.
