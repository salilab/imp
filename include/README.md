Place the public header files in this directory. They will be
available to your code (and other modules with

     #include <IMP/scratch/myheader.h>

All headers should include `IMP/scratch/scratch_config.h` as their
first include and surround all code with `IMPSCRATCH_BEGIN_NAMESPACE`
and `IMP_SCRATCH_END_NAMESPACE` to put it in the scratch namespace and
manage compiler warnings.

Headers should also be exposed to swig in the `pyext/swig.i-in` file.