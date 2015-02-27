This directory is to be used for module-specific utility programs, typically
used during the build procedure. These programs are *not* installed so will
not be seen by the typical end users of your module (for that, use the `bin`
directory).

Place source `.cpp` files and `.py` files here to be built into
executables. Each file will result in one executable. The `.cpp` files
will be linked against `libimp_scratch` and all its dependencies. Each
executable should use IMP::base::setup_from_argv() to process command
line flags.
