CROSS_BIN=$(realpath ../cross-compiler/i386-elf/bin)
PYTHON_BIN=$(realpath ../python-3.11.9/bin)
PYTHON_LIB=$(realpath ../python-3.11.9/lib)

export PATH=$PYTHON_BIN:$CROSS_BIN:$PATH
export LD_LIBRARY_PATH=$PYTHON_LIB:$LD_LIBRARY_PATH
