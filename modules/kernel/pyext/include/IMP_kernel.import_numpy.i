// If IMP is built with numpy, initialize it in every module

#if IMP_KERNEL_HAS_NUMPY
%begin %{
static int numpy_import_retval;
%}

%{
#include <numpy/arrayobject.h>
%}

%init {
  numpy_import_retval = _import_array();
  /* If numpy was not found, continue anyway without numpy support */
  PyErr_Clear();
}
#endif
