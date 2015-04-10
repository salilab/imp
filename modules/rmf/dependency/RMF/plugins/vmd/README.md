VMD Plugin {#vmdplugin}
==========

# Building and installing #
[TOC]

Once you build RMF, there will be a file called `rmfplugin.so` in the
`lib` directory. Creating a link to that in the
`vmd/plugins/YOUR_ARCHITECTURE/molfile/` directory, should make `vmd`
load the plugin next time it launches.

If you can't modify your VMD build's plugin directory, you can load the
plugin by adding something like `vmd_plugin_scandirectory /Users/drussel/src/rmf/32_bit/lib/ rmfplugin.so`
to your `.vmdrc`. Note, you probably also want to have the line `menu main on`
to restore default behavior of VMD. Note also, this method does not allow
you to open models from the command line. You have to launch VMD and then do
`New Molecule...`.


## Building on Mac OS ##
VMD doesn't seem to support 64 bit builds on MacOS (at least no usable
pre-built ones are provided). By default, RMF is build 64 bit, and so
the plugin will not work. To build a 32 bit version with
[HomeBrew](http://brew.sh) do
- build `szip`, `hdf5`, `log4cxx` and `boost` with the `--universal` flag
  (you have to uninstall them first).
- configure RMF with `-DCMAKE_OSX_ARCHITECTURES=i386`
- build it and link as above


# Caveats #
- If your version of VMD is not 1.9.1, pass the correct path for
  finding the headers to cmake:
  `-DVMDPLUGIN_INCLUDE_DIR=/path/to/vmd/plugin/include`. The directory
  is the one that contains `molfile_plugin.h`.
- VMD's plugin model supports static geometry and atoms (or pseudo
  atoms) that change with each frame. So no moving cylinders. Balls
  are mapped to atoms.
- the plugin is linked against `libRMF` and `Avro` in the build directory
- it is not been tested on windows
