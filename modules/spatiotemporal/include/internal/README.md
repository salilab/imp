Place the private header files in this directory. They will be
available to your code with

     #include <IMP/spatiotemporal/internal/myheader.h>

All headers should include `IMP/spatiotemporal/spatiotemporal_config.h` as their
first include and surround all code with `IMPSPATIOTEMPORAL_BEGIN_INTERNAL_NAMESPACE`
and `IMPSPATIOTEMPORAL_END_INTERNAL_NAMESPACE` to put it in the
IMP::spatiotemporal::internal namespace and manage compiler warnings.
