Place source `.cpp` files and `.py` files here to be built into
executables. Each file will result in one executable. The `.cpp` files
will be linked against `libimp_scratch` and all its dependencies. Each
executable should use IMP::base::setup_from_argv() to process command
line flags.