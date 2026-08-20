// Allow accessing model data via NumPy


%{
PyObject *_get_floats_data_numpy(PyObject *m_pyobj, unsigned sz, double *data,
                                 bool read_only)
{
#if IMP_KERNEL_HAS_NUMPY
  if (numpy_import_retval != 0) {
    PyErr_SetString(PyExc_ImportError,
                    "IMP's NumPy support did not initialize correctly");
    return NULL;
  }

  npy_intp dims[1];
  dims[0] = sz;

  /* Note that attribute tables are C-style contiguous so no special strides or
     other flags need to be passed to NumPy */
  PyObject *obj = PyArray_New(&PyArray_Type, 1, dims, NPY_DOUBLE, NULL,
                              data, 0, read_only ? 0 : NPY_ARRAY_WRITEABLE,
                              NULL);
  if (!obj) {
    return NULL;
  }

  /* Ensure that the Model is kept around as long as the numpy object
     is alive. */
  Py_INCREF(m_pyobj);
  if (PyArray_SetBaseObject((PyArrayObject *)obj, m_pyobj) != 0) {
    Py_DECREF(m_pyobj);
    Py_DECREF(obj);
    return NULL;
  }

  return obj;
#else
  PyErr_SetString(PyExc_NotImplementedError,
                  "IMP was built without NumPy support");
  return NULL;
#endif
}

PyObject *_get_ints_data_numpy(PyObject *m_pyobj, unsigned sz, int *data,
                               bool read_only)
{
#if IMP_KERNEL_HAS_NUMPY
  if (numpy_import_retval != 0) {
    PyErr_SetString(PyExc_ImportError,
                    "IMP's NumPy support did not initialize correctly");
    return NULL;
  }

  npy_intp dims[1];
  dims[0] = sz;

  /* Note that attribute tables are C-style contiguous so no special strides or
     other flags need to be passed to NumPy */
  PyObject *obj = PyArray_New(&PyArray_Type, 1, dims, NPY_INT32, NULL,
                              data, 0, read_only ? 0 : NPY_ARRAY_WRITEABLE,
                              NULL);
  if (!obj) {
    return NULL;
  }

  /* Ensure that the Model is kept around as long as the numpy object
     is alive. */
  Py_INCREF(m_pyobj);
  if (PyArray_SetBaseObject((PyArrayObject *)obj, m_pyobj) != 0) {
    Py_DECREF(m_pyobj);
    Py_DECREF(obj);
    return NULL;
  }

  return obj;
#else
  PyErr_SetString(PyExc_NotImplementedError,
                  "IMP was built without NumPy support");
  return NULL;
#endif
}

PyObject *_get_vector3ds_data_numpy(PyObject *m_pyobj, unsigned sz,
                                    Vector3D *data, bool read_only)
{
#if IMP_KERNEL_HAS_NUMPY
  if (numpy_import_retval != 0) {
    PyErr_SetString(PyExc_ImportError,
                    "IMP's NumPy support did not initialize correctly");
    return NULL;
  }

  npy_intp dims[2];
  dims[0] = sz;
  dims[1] = 3;

  static_assert(sizeof(Vector3D) == 3 * sizeof(double),
                "Vector3D size != 3 * double size");
  PyObject *obj = PyArray_New(&PyArray_Type, 2, dims, NPY_DOUBLE, NULL,
                              data, 0, read_only ? 0 : NPY_ARRAY_WRITEABLE,
                              NULL);
  if (!obj) {
    return NULL;
  }

  /* Ensure that the Model is kept around as long as the numpy object
     is alive. */
  Py_INCREF(m_pyobj);
  if (PyArray_SetBaseObject((PyArrayObject *)obj, m_pyobj) != 0) {
    Py_DECREF(m_pyobj);
    Py_DECREF(obj);
    return NULL;
  }

  return obj;
#else
  PyErr_SetString(PyExc_NotImplementedError,
                  "IMP was built without NumPy support");
  return NULL;
#endif
}

PyObject *_get_vector4ds_data_numpy(PyObject *m_pyobj, unsigned sz,
                                    Vector4D *data, bool read_only)
{
#if IMP_KERNEL_HAS_NUMPY
  if (numpy_import_retval != 0) {
    PyErr_SetString(PyExc_ImportError,
                    "IMP's NumPy support did not initialize correctly");
    return NULL;
  }

  npy_intp dims[2];
  dims[0] = sz;
  dims[1] = 4;

  static_assert(sizeof(Vector4D) == 4 * sizeof(double),
                "Vector4D size != 4 * double size");
  PyObject *obj = PyArray_New(&PyArray_Type, 2, dims, NPY_DOUBLE, NULL,
                              data, 0, read_only ? 0 : NPY_ARRAY_WRITEABLE,
                              NULL);
  if (!obj) {
    return NULL;
  }

  /* Ensure that the Model is kept around as long as the numpy object
     is alive. */
  Py_INCREF(m_pyobj);
  if (PyArray_SetBaseObject((PyArrayObject *)obj, m_pyobj) != 0) {
    Py_DECREF(m_pyobj);
    Py_DECREF(obj);
    return NULL;
  }

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
                                 PyObject *tuple, Py_ssize_t pos,
                                 bool read_only)
{
  PyObject *obj = PyArray_New(&PyArray_Type, nd, dims, NPY_DOUBLE, strides,
                              data, 0, read_only ? 0 : NPY_ARRAY_WRITEABLE,
                              NULL);
  if (!obj) {
    Py_DECREF(tuple);
    return NULL;
  }

  if (PyTuple_SetItem(tuple, pos, obj) == 0) {
    /* Ensure that the Model is kept around as long as the numpy object
       is alive. */
    Py_INCREF(m_pyobj);
    if (PyArray_SetBaseObject((PyArrayObject *)obj, m_pyobj) != 0) {
      Py_DECREF(m_pyobj);
      Py_DECREF(obj);
      Py_DECREF(tuple);
      return NULL;
    }
    return obj;
  } else {
    Py_DECREF(obj);
    Py_DECREF(tuple);
    return NULL;
  }
}
#endif

PyObject *_get_spheres_data_numpy(PyObject *m_pyobj, unsigned sz,
                                  algebra::Sphere3D *data, bool read_only)
{
#if IMP_KERNEL_HAS_NUMPY
  if (numpy_import_retval != 0) {
    PyErr_SetString(PyExc_ImportError,
                    "IMP's NumPy support did not initialize correctly");
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
  if (!_add_spheres_component(pt, 2, dims, strides, m_pyobj, tuple, 0,
                              read_only)) {
    return NULL;
  }

  /* r */
  pt = (char *)data;
  if (data) pt += radius_offset;
  if (!_add_spheres_component(pt, 1, dims, strides, m_pyobj, tuple, 1,
                              read_only)) {
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
                                 PyObject *m_pyobj, bool read_only)
{
  unsigned sz = m->get_derivative_size(k);
  return _get_floats_data_numpy(m_pyobj, sz,
                             sz == 0 ? nullptr : m->access_derivative_data(k),
                             read_only);
}

PyObject *_get_floats_numpy(IMP::Model *m, IMP::FloatKey k, PyObject *m_pyobj,
                            bool read_only)
{
  unsigned sz = m->IMP::internal::FloatAttributeTable::get_attribute_size(k);
  return _get_floats_data_numpy(m_pyobj, sz,
           sz == 0 ? nullptr
             : m->IMP::internal::FloatAttributeTable::access_attribute_data(k),
           read_only);
}

PyObject *_get_ints_numpy(IMP::Model *m, IMP::IntKey k, PyObject *m_pyobj,
                          bool read_only)
{
  unsigned sz = m->IMP::internal::IntAttributeTable::get_attribute_size(k);
  return _get_ints_data_numpy(m_pyobj, sz,
           sz == 0 ? nullptr
               : m->IMP::internal::IntAttributeTable::access_attribute_data(k),
           read_only);
}

PyObject *_get_particle_indexes_numpy(IMP::Model *m, IMP::ParticleIndexKey k,
                                      PyObject *m_pyobj, bool read_only)
{
  unsigned sz = m->IMP::internal::ParticleAttributeTable::get_attribute_size(k);
  // Particle indexes are just integers
  static_assert(sizeof(ParticleIndex) == sizeof(int));
  return _get_ints_data_numpy(m_pyobj, sz,
   sz == 0 ? nullptr
    : (int *)m->IMP::internal::ParticleAttributeTable::access_attribute_data(k),
   read_only);
}

PyObject *_get_vector3ds_numpy(IMP::Model *m, IMP::Vector3DKey k,
                               PyObject *m_pyobj, bool read_only)
{
  unsigned sz = m->IMP::internal::Vector3DAttributeTable::get_attribute_size(k);
  return _get_vector3ds_data_numpy(m_pyobj, sz,
       sz == 0 ? nullptr
        : m->IMP::internal::Vector3DAttributeTable::access_attribute_data(k),
       read_only);
}

PyObject *_get_vector3dderiv_numpy(IMP::Model *m, IMP::Vector3DDerivKey k,
                                   PyObject *m_pyobj, bool read_only)
{
  unsigned sz
      = m->IMP::internal::Vector3DDerivAttributeTable::get_attribute_size(k);
  return _get_vector3ds_data_numpy(m_pyobj, sz,
    sz == 0 ? nullptr
      : m->IMP::internal::Vector3DDerivAttributeTable::access_attribute_data(k),
    read_only);
}

PyObject *_get_vector3dderiv_derivatives_numpy(
               IMP::Model *m, IMP::Vector3DDerivKey k,
               PyObject *m_pyobj, bool read_only)
{
  unsigned sz
      = m->IMP::internal::Vector3DDerivAttributeTable::get_derivative_size(k);
  return _get_vector3ds_data_numpy(m_pyobj, sz,
    sz == 0 ? nullptr
     : m->IMP::internal::Vector3DDerivAttributeTable::access_derivative_data(k),
    read_only);
}

PyObject *_get_vector4ds_numpy(IMP::Model *m, IMP::Vector4DKey k,
                               PyObject *m_pyobj, bool read_only)
{
  unsigned sz = m->IMP::internal::Vector4DAttributeTable::get_attribute_size(k);
  return _get_vector4ds_data_numpy(m_pyobj, sz,
       sz == 0 ? nullptr
        : m->IMP::internal::Vector4DAttributeTable::access_attribute_data(k),
       read_only);
}

PyObject *_get_vector4dderiv_numpy(IMP::Model *m, IMP::Vector4DDerivKey k,
                                   PyObject *m_pyobj, bool read_only)
{
  unsigned sz
      = m->IMP::internal::Vector4DDerivAttributeTable::get_attribute_size(k);
  return _get_vector4ds_data_numpy(m_pyobj, sz,
    sz == 0 ? nullptr
      : m->IMP::internal::Vector4DDerivAttributeTable::access_attribute_data(k),
    read_only);
}

PyObject *_get_vector4dderiv_derivatives_numpy(
               IMP::Model *m, IMP::Vector4DDerivKey k,
               PyObject *m_pyobj, bool read_only)
{
  unsigned sz
      = m->IMP::internal::Vector4DDerivAttributeTable::get_derivative_size(k);
  return _get_vector4ds_data_numpy(m_pyobj, sz,
    sz == 0 ? nullptr
     : m->IMP::internal::Vector4DDerivAttributeTable::access_derivative_data(k),
    read_only);
}

PyObject *_get_spheres_numpy(IMP::Model *m, PyObject *m_pyobj, bool read_only)
{
  unsigned sz = m->get_spheres_size();
  return _get_spheres_data_numpy(m_pyobj, sz,
                   sz == 0 ? nullptr : m->access_spheres_data(), read_only);
}

PyObject *_get_sphere_derivatives_numpy(IMP::Model *m, PyObject *m_pyobj,
                                        bool read_only)
{
  unsigned sz = m->get_sphere_derivatives_size();
  return _get_spheres_data_numpy(m_pyobj, sz,
                  sz == 0 ? nullptr : m->access_sphere_derivatives_data(),
                  read_only);
}

PyObject *_get_internal_coordinates_numpy(IMP::Model *m, PyObject *m_pyobj,
                                          bool read_only)
{
  unsigned sz = m->get_internal_coordinates_size();
  return _get_vector3ds_data_numpy(m_pyobj, sz,
       sz == 0 ? nullptr : m->access_internal_coordinates_data(), read_only);
}

PyObject *_get_internal_coordinate_derivatives_numpy(
                IMP::Model *m, PyObject *m_pyobj, bool read_only)
{
  unsigned sz = m->get_internal_coordinate_derivatives_size();
  return _get_vector3ds_data_numpy(m_pyobj, sz,
       sz == 0 ? nullptr : m->access_internal_coordinate_derivatives_data(),
       read_only);
}
%}

%extend IMP::Model {
  %pythoncode %{
    def get_ints_numpy(self, k, read_only=False):
        """Get the model's attribute array for IntKey k as a NumPy array.
           The array is indexed by ParticleIndex; particles that don't have
           this attribute will either be off the end of the array or will have
           the value INT_MAX.
           This is a NumPy view that shares memory with the Model. Thus,
           any changes to values in this list will be reflected in the Model.
           Also, if the Model attribute array moves in memory (e.g. if particles
           or attributes are added) this array will be invalidated, so it is
           unsafe to keep it around long term.
           If read_only is set True, values in the array cannot be changed.
        """
        return _get_ints_numpy(self, k, self, read_only)

    def get_numpy(self, k, read_only=False):
        """Get the model's attribute array for any type of Key
           k as a NumPy array. See Model::get_ints_numpy() for more details."""
        _numpy_meth_map = {IntKey: _get_ints_numpy,
                           FloatKey: _get_floats_numpy,
                           Vector3DKey: _get_vector3ds_numpy,
                           Vector3DDerivKey: _get_vector3dderiv_numpy,
                           Vector4DKey: _get_vector4ds_numpy,
                           Vector4DDerivKey: _get_vector4dderiv_numpy,
                           ParticleIndexKey: _get_particle_indexes_numpy}
        return _numpy_meth_map[type(k)](self, k, self, read_only)

    def get_floats_numpy(self, k, read_only=False):
        """Get the model's attribute array for FloatKey k as a NumPy array.
           See Model::get_ints_numpy() for more details."""
        return _get_floats_numpy(self, k, self, read_only)

    def get_derivatives_numpy(self, k, read_only=False):
        """Get the model's attribute derivatives array for key k
           as a NumPy array. See Model::get_ints_numpy() for more details."""
        _numpy_meth_map = {
            FloatKey: _get_derivatives_numpy,
            Vector3DDerivKey: _get_vector3dderiv_derivatives_numpy,
            Vector4DDerivKey: _get_vector4dderiv_derivatives_numpy}
        return _numpy_meth_map[type(k)](self, k, self, read_only)

    def get_vector3ds_numpy(self, k, read_only=False):
        """Get the model's attribute array for Vector3DKey k as a NumPy array.
           See Model::get_ints_numpy() for more details."""
        return _get_vector3ds_numpy(self, k, self, read_only)

    def get_spheres_numpy(self, read_only=False):
        """Get the model's XYZR attribute arrays as NumPy arrays.
           The attribute arrays for Cartesian coordinates and radii are
           stored separately from those for other FloatKeys. This function
           returns a tuple of two NumPy arrays, the first of coordinates and
           the second of radii. See Model::get_ints_numpy() for more details."""
        return _get_spheres_numpy(self, self, read_only)

    def get_sphere_derivatives_numpy(self, read_only=False):
        """Get the model's XYZR attribute derivatives arrays as NumPy arrays.
           See Model::get_ints_numpy() for more details."""
        return _get_sphere_derivatives_numpy(self, self, read_only)

    def get_internal_coordinates_numpy(self, read_only=False):
        """Get the model's internal coordinate array as a NumPy array.
           The attribute arrays for rigid body internal coordinates are
           stored separately from those for other FloatKeys.
           See Model::get_ints_numpy() for more details."""
        return _get_internal_coordinates_numpy(self, self, read_only)

    def get_internal_coordinate_derivatives_numpy(self, read_only=False):
        """Get the model's internal coordinate derivative array as a
           NumPy array.
           See Model::get_ints_numpy() for more details."""
        return _get_internal_coordinate_derivatives_numpy(
            self, self, read_only)
  %}
}

// Always treat particle indexes as numpy.intc when using numpy
#if IMP_KERNEL_HAS_NUMPY
%pythoncode %{
try:
    import numpy
    ParticleIndex = numpy.intc
except ImportError:
    pass
%}
#endif
