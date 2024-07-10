Dependencies:

1) cross-compiler (write some notes on how to install it)
2) python compiled with --enable-shared. GDB depends on python dynamic library and not on python itself.
3) mtools for managing FAT32 disk images without mounting them. I use this since mounting require root permissions

Install Python 3.11. Because my GDB dependant on this exact version. On my machine I do something like this in Python3.11 sources directory:
```
./configure --prefix="/home/khasan/code/osdev/python-3.11.9" --enable-optimizations --with-lto --with-ensurepip=upgrade --enable-shared --with-system-expat --with-system-ffi --enable-ipv6 && make -j8 && make install
```
I also did something like because had some error
`ln -s libpython3.11d.so libpython3.11.so.1.0`


Run
`source env.sh`


Memory Map:
0x0000-0x7BFF      some BIOS stuff \
0x7C00-0x7DFF      my boot.asm (512 bytes) \
0x07E00-0x7FFFF    memory for my kernel (480.5 KiB) \
0x80000-0xFFFFF    some BIOS stuff (including VGA mapped memory) \
0x100000-0x1fffff  kernel stack (1 MiB) (grows backward) \
0x200000-END       kernel heap \


Some links:
Bios memory map https://wiki.osdev.org/Memory_Map_(x86)
FAT32 tutorial https://www.pjrc.com/tech/8051/ide/fat32.html
