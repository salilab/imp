// If IMP is built with numpy, initialize it in every module

#if IMP_KERNEL_HAS_NUMPY
%begin %{
#include <numpy/arrayobject.h>

static int numpy_import_retval;
%}

%init {
  numpy_import_retval = _import_array();
}
#endif
