This directory is to be used for command line tools to be used with this module.
They will be installed so that end users of IMP and your module can use them
(for utility programs that are *not* installed, use the `utility` directory).

Note that the binary directory on a typical computer is a single namespace,
so you should:

 - be careful not to litter this directory with a large number of programs.
   One solution is to use a single 'driver' program that has multiple modes
   (like the `git` or `svn` command line tools). IMP provides an
   IMP::CommandDispatcher Python class to facilitate this (see IMP::multifit
   for an example of it in action).

 - take care to give each program a fairly unique name. One solution is to
   use a module-specific prefix. For an example, see IMP::rmf which provides
   `rmf_display`, `rmf_simplify` etc.

Place source `.cpp` files and Python files (without extension) here to be
built into executables. Each file will result in one executable. The
`.cpp` files will be linked against `libimp_scratch` and all its dependencies.
Each executable should use IMP::base::setup_from_argv() to process command
line flags.
