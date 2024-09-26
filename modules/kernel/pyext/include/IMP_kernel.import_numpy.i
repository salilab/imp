// If IMP is built with numpy, initialize it in every module

#if IMP_KERNEL_HAS_NUMPY
%begin %{
static int numpy_import_retval;
%}

%{
// Silence warnings about old NumPy API
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
%}

%init {
  numpy_import_retval = _import_array();
  /* If numpy was not found, continue anyway without numpy support, but warn
     the user */
  PyErr_Clear();
  if (numpy_import_retval < 0) {
    PyErr_WarnEx(PyExc_RuntimeWarning,
        "IMP's NumPy support did not initialize correctly. Some NumPy-related "
        "functionality will be unavailable. This is usually caused by "
        "building IMP with a newer version of NumPy than is available "
        "at runtime.", 1);
  }
}
#endif
