Dependencies:

1) cross-compiler (write some notes on how to install it)
2) python compiled with --enable-shared. GDB depends on python dynamic library and not on python itself.

Install Python 3.11. Because my GDB dependant on this exact version
```
./configure --with-pydebug --prefix="<PROJECT>/python-3.11.9" --enable-optimizations --with-lto --with-ensurepip=upgrade --enable-shared --with-system-expat --with-system-ffi --enable-ipv6
make -j 8
make install
```
I also did something like because had some error
`ln -s libpython3.11d.so libpython3.11.so.1.0`


Run
`source env.sh`
