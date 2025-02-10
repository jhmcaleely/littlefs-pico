# littlefs-pico

This repo contains two implementations of the littlefs sample code. One runs on the pico, and demonstrates sharing the flash device between firmware and a flash filesystem. The other runs on your host pc, and demonstrates creating pre-populated filesystems on the host, for subsequent flashing to the pico. A filesystem image can be combined with a firmware image to produce a single flashable uf2 file.

pico/ Firmware for Pico that runs the littlefs sample on target
host-uf2/ Host PC program to create uf2 files with littlefs sample

littlefs/ Upstream littlefs
uf2/ Upstream UF2

pico_flash_fs.h Shared definitions of a flash filesystem stoarage area.