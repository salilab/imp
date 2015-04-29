Place the private header files in this directory. They will be
available to your code with

     #include <IMP/scratch/internal/myheader.h>

All headers should include `IMP/scratch/scratch_config.h` as their
first include and surround all code with `IMPSCRATCH_BEGIN_INTERNAL_NAMESPACE`
and `IMP_SCRATCH_END_INTERNAL_NAMESPACE` to put it in the scratch namespace and
manage compiler warnings.
