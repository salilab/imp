// Allow accessing model data via NumPy


%{
PyObject *_get_floats_data_numpy(PyObject *m_pyobj, unsigned sz, double *data)
{
#if IMP_KERNEL_HAS_NUMPY
  if (numpy_import_retval != 0) {
    return NULL;
  }

  npy_intp dims[1];
  dims[0] = sz;

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

PyObject *_get_ints_data_numpy(PyObject *m_pyobj, unsigned sz, int *data)
{
#if IMP_KERNEL_HAS_NUMPY
  if (numpy_import_retval != 0) {
    return NULL;
  }

  npy_intp dims[1];
  dims[0] = sz;

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

#if IMP_KERNEL_HAS_NUMPY
PyObject *_add_spheres_component(void *data, int nd, npy_intp *dims,
                                 npy_intp *strides, PyObject *m_pyobj,
                                 PyObject *tuple, Py_ssize_t pos)
{
  PyObject *obj = PyArray_New(&PyArray_Type, nd, dims, NPY_DOUBLE, strides,
                              data, 0, NPY_WRITEABLE, NULL);
  if (!obj) {
    Py_DECREF(tuple);
    return NULL;
  }

  if (PyTuple_SetItem(tuple, pos, obj) == 0) {
    /* Ensure that the Model is kept around as long as the numpy object
       is alive. */
    Py_INCREF(m_pyobj);
    PyArray_BASE(obj) = m_pyobj;
    return obj;
  } else {
    Py_DECREF(obj);
    Py_DECREF(tuple);
    return NULL;
  }
}
#endif

PyObject *_get_spheres_data_numpy(PyObject *m_pyobj, unsigned sz,
                                  algebra::Sphere3D *data)
{
#if IMP_KERNEL_HAS_NUMPY
  if (numpy_import_retval != 0) {
    return NULL;
  }

  size_t struct_size, center_offset, radius_offset;
  algebra::Sphere3D::_get_struct_size(struct_size, center_offset,
                                      radius_offset);

  npy_intp dims[2], strides[2];
  dims[0] = sz;
  strides[0] = struct_size;

  PyObject *tuple = PyTuple_New(2);
  if (!tuple) {
    return NULL;
  }

  /* x, y, z */
  char *pt = (char *)data;
  if (data) pt += center_offset;
  dims[1] = 3;
  strides[1] = sizeof(double);
  if (!_add_spheres_component(pt, 2, dims, strides, m_pyobj, tuple, 0)) {
    return NULL;
  }

  /* r */
  pt = (char *)data;
  if (data) pt += radius_offset;
  if (!_add_spheres_component(pt, 1, dims, strides, m_pyobj, tuple, 1)) {
    return NULL;
  }
  return tuple;
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
  unsigned sz = m->get_derivative_size(k);
  return _get_floats_data_numpy(m_pyobj, sz,
                             sz == 0 ? nullptr : m->access_derivative_data(k));
}

PyObject *_get_floats_numpy(IMP::Model *m, IMP::FloatKey k, PyObject *m_pyobj)
{
  unsigned sz = m->IMP::internal::FloatAttributeTable::get_attribute_size(k);
  return _get_floats_data_numpy(m_pyobj, sz,
           sz == 0 ? nullptr
             : m->IMP::internal::FloatAttributeTable::access_attribute_data(k));
}

PyObject *_get_ints_numpy(IMP::Model *m, IMP::IntKey k, PyObject *m_pyobj)
{
  unsigned sz = m->IMP::internal::IntAttributeTable::get_attribute_size(k);
  return _get_ints_data_numpy(m_pyobj, sz,
           sz == 0 ? nullptr
               : m->IMP::internal::IntAttributeTable::access_attribute_data(k));
}

PyObject *_get_spheres_numpy(IMP::Model *m, PyObject *m_pyobj)
{
  unsigned sz = m->get_spheres_size();
  return _get_spheres_data_numpy(m_pyobj, sz,
                   sz == 0 ? nullptr : m->access_spheres_data());
}

PyObject *_get_sphere_derivatives_numpy(IMP::Model *m, PyObject *m_pyobj)
{
  unsigned sz = m->get_sphere_derivatives_size();
  return _get_spheres_data_numpy(m_pyobj, sz,
                  sz == 0 ? nullptr : m->access_sphere_derivatives_data());
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

// Always treat particle indexes as numpy.int32 when using numpy
#ifdef IMP_KERNEL_HAS_NUMPY
%pythoncode %{
try:
    import numpy
    ParticleIndex = numpy.int32
except ImportError:
    pass
%}
#endif
