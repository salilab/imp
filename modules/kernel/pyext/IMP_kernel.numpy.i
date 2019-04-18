// Allow accessing model data via NumPy


%{
#if IMP_KERNEL_HAS_NUMPY
#include <numpy/arrayobject.h>

static bool import_numpy_module()
{
  static bool imported;

  if (imported) {
    return true;
  } else {
    int ret = _import_array();
    if (ret == 0) {
      imported = true;
    }
    return (ret == 0);
  }
}
#endif

PyObject *_get_floats_data_numpy(IMP::Model *m, PyObject *m_pyobj, double *data)
{
#if IMP_KERNEL_HAS_NUMPY
  if (!import_numpy_module()) {
    return NULL;
  }

  npy_intp dims[1];
  dims[0] = m->get_number_of_particle_indexes();

  /* Note that attribute tables are C-style contiguous so no special strides or
     other flags need to be passed to NumPy */
  PyObject *obj = PyArray_New(&PyArray_Type, 1, dims, NPY_DOUBLE, NULL,
                              data, 0, NPY_WRITEABLE, NULL);
  if (!obj) {
    return NULL;
  }

  /* Ensure that the Model is kept around as long as the numpy object
     is alive. */
  Py_INCREF(m_pyobj);
  PyArray_BASE(obj) = m_pyobj;

  return obj;
#else
  PyErr_SetString(PyExc_NotImplementedError,
                  "IMP was built without NumPy support");
  return NULL;
#endif
}

PyObject *_get_ints_data_numpy(IMP::Model *m, PyObject *m_pyobj, int *data)
{
#if IMP_KERNEL_HAS_NUMPY
  if (!import_numpy_module()) {
    return NULL;
  }

  npy_intp dims[1];
  dims[0] = m->get_number_of_particle_indexes();

  /* Note that attribute tables are C-style contiguous so no special strides or
     other flags need to be passed to NumPy */
  PyObject *obj = PyArray_New(&PyArray_Type, 1, dims, NPY_INT32, NULL,
                              data, 0, NPY_WRITEABLE, NULL);
  if (!obj) {
    return NULL;
  }

  /* Ensure that the Model is kept around as long as the numpy object
     is alive. */
  Py_INCREF(m_pyobj);
  PyArray_BASE(obj) = m_pyobj;

  return obj;
#else
  PyErr_SetString(PyExc_NotImplementedError,
                  "IMP was built without NumPy support");
  return NULL;
#endif
}


PyObject *_get_spheres_data_numpy(IMP::Model *m, PyObject *m_pyobj,
                                  algebra::Sphere3D *data)
{
#if IMP_KERNEL_HAS_NUMPY
  if (!import_numpy_module()) {
    return NULL;
  }

  // We treat an array of N spheres as a 4*N 2D array, so make sure the
  // internal layout of the Sphere3D class matches this assumption
  BOOST_STATIC_ASSERT(sizeof(algebra::Sphere3D) == 4 * sizeof(double));

  npy_intp dims[2];
  dims[0] = m->get_number_of_particle_indexes();
  dims[1] = 4;

  PyObject *obj = PyArray_New(&PyArray_Type, 2, dims, NPY_DOUBLE, NULL,
                              data, 0, NPY_WRITEABLE, NULL);
  if (!obj) {
    return NULL;
  }

  /* Ensure that the Model is kept around as long as the numpy object
     is alive. */
  Py_INCREF(m_pyobj);
  PyArray_BASE(obj) = m_pyobj;

  return obj;
#else
  PyErr_SetString(PyExc_NotImplementedError,
                  "IMP was built without NumPy support");
  return NULL;
#endif
}

%}

%inline %{
PyObject *_get_derivatives_numpy(IMP::Model *m, IMP::FloatKey k,
                                 PyObject *m_pyobj)
{
  return _get_floats_data_numpy(m, m_pyobj, m->access_derivative_data(k));
}

PyObject *_get_floats_numpy(IMP::Model *m, IMP::FloatKey k, PyObject *m_pyobj)
{
  return _get_floats_data_numpy(m, m_pyobj,
               m->IMP::internal::FloatAttributeTable::access_attribute_data(k));
}

PyObject *_get_ints_numpy(IMP::Model *m, IMP::IntKey k, PyObject *m_pyobj)
{
  return _get_ints_data_numpy(m, m_pyobj,
               m->IMP::internal::IntAttributeTable::access_attribute_data(k));
}

PyObject *_get_spheres_numpy(IMP::Model *m, PyObject *m_pyobj)
{
  return _get_spheres_data_numpy(m, m_pyobj, m->access_spheres_data());
}

PyObject *_get_sphere_derivatives_numpy(IMP::Model *m, PyObject *m_pyobj)
{
  return _get_spheres_data_numpy(m, m_pyobj,
                                 m->access_sphere_derivatives_data());
}
%}

%extend IMP::Model {
  %pythoncode %{
    def _get_ints_numpy(self, k):
        return _get_ints_numpy(self, k, self)

    def _get_floats_numpy(self, k):
        return _get_floats_numpy(self, k, self)

    def _get_derivatives_numpy(self, k):
        return _get_derivatives_numpy(self, k, self)

    def _get_spheres_numpy(self):
        return _get_spheres_numpy(self, self)

    def _get_sphere_derivatives_numpy(self):
        return _get_sphere_derivatives_numpy(self, self)
  %}
}
