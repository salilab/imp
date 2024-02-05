Place the public header files in this directory. They will be
available to your code (and other modules) with

     #include <IMP/spatiotemporal/myheader.h>

All headers should include `IMP/spatiotemporal/spatiotemporal_config.h` as their
first include and surround all code with `IMPSPATIOTEMPORAL_BEGIN_NAMESPACE`
and `IMPSPATIOTEMPORAL_END_NAMESPACE` to put it in the IMP::spatiotemporal namespace
and manage compiler warnings.

Headers should also be exposed to SWIG in the `pyext/swig.i-in` file.
