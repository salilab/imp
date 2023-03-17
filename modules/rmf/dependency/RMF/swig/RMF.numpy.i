#if RMF_HAS_NUMPY
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
  /* If numpy was not found, continue anyway without numpy support */
  PyErr_Clear();
}

%{
// Return true iff `o` is a numpy array of the given type laid out in
// a contiguous chunk of memory
bool is_native_numpy_array(PyObject *o, int numpy_type) {
  if (!o || !PyArray_Check(o)) { return false; }// not a numpy array

  PyArrayObject *a = (PyArrayObject *)o;
  int array_type = PyArray_TYPE(a);
  if (array_type != NPY_NOTYPE
      && !PyArray_EquivTypenums(array_type, numpy_type)) {
    return false;  // data type does not match
  }

  return PyArray_ISCONTIGUOUS(a) && PyArray_ISNOTSWAPPED(a);
}

// Return true iff `o` is a 2D numpy array of the given type laid out in
// a contiguous chunk of memory
bool is_native_numpy_2d_array(PyObject *o, int numpy_type, npy_intp ncol) {
  if (is_native_numpy_array(o, numpy_type)) {
    PyArrayObject *a = (PyArrayObject *)o;
    return PyArray_NDIM(a) == 2 && PyArray_DIM(a, 1) == ncol;
  } else {
    return false;
  }
}

// Utility class to visit RMF nodes and extract XYZ coordinates
class _OurVisitor {
  RMF::decorator::ReferenceFrameConstFactory refframef_;
  RMF::decorator::ParticleConstFactory particlef_;
  RMF::decorator::BallConstFactory ballf_;
  RMF::decorator::AlternativesConstFactory altf_;
  // Dimension of the NumPy array
  npy_intp ncoord_;
  // Raw data in the N*3 NumPy array
  double *data_;

  void add_coordinates(const RMF::Vector3 &v) {
    numxyz++;
    if (numxyz > ncoord_) {
      std::ostringstream oss;
      oss << "More XYZ particles were found in the RMF file than "
          << "were provided (" << ncoord_ << ") in the numpy array";
      throw std::domain_error(oss.str());
    }
    *data_++ = v[0];
    *data_++ = v[1];
    *data_++ = v[2];
  }

public:
  _OurVisitor(RMF::FileConstHandle fh, npy_intp ncoord, double *data)
          : refframef_(fh), particlef_(fh), ballf_(fh), altf_(fh),
            ncoord_(ncoord), data_(data) {}

  void handle_node(RMF::NodeConstHandle &nh, RMF::CoordinateTransformer tran) {
    if (refframef_.get_is(nh)) {
      tran = RMF::CoordinateTransformer(tran, refframef_.get(nh));
    }
    if (ballf_.get_is(nh)) {
      RMF::Vector3 coord = tran.get_global_coordinates(
                               ballf_.get(nh).get_coordinates());
      add_coordinates(coord);
    } else if (particlef_.get_is(nh)) {
      RMF::Vector3 coord = tran.get_global_coordinates(
                               particlef_.get(nh).get_coordinates());
      add_coordinates(coord);
    }
    for (auto &child : nh.get_children()) {
      handle_node(child, tran);
    }
    if (altf_.get_is(nh)) {
      RMF::decorator::AlternativesConst alt = altf_.get(nh);
      RMF::NodeConstHandles altp = alt.get_alternatives(RMF::PARTICLE);
      // Skip first element (already handled above by get_children())
      for (auto altpi = altp.begin() + 1; altpi < altp.end(); ++altpi) {
        handle_node(*altpi, tran);
      }
      for (auto &childg : alt.get_alternatives(RMF::GAUSSIAN_PARTICLE)) {
        handle_node(childg, tran);
      }
    }
  }

  size_t numxyz = 0;
};

%}

%inline %{
void get_all_global_coordinates(
     RMF::FileConstHandle &fh, RMF::NodeConstHandle &nh, PyObject *coord) {
  if (numpy_import_retval != 0) {
    throw std::runtime_error("NumPy did not initialize");
  }
  if (!is_native_numpy_2d_array(coord, NPY_DOUBLE, 3)) {
    throw std::invalid_argument("NumPy array is not a native N*3 double array");
  }

  PyArrayObject *acoord = (PyArrayObject *)coord;
  npy_intp ncoord = PyArray_DIM(acoord, 0);
  double *data = (double *)PyArray_DATA(acoord);

  _OurVisitor v(fh, ncoord, data);
  v.handle_node(nh, RMF::CoordinateTransformer());
  if (v.numxyz != ncoord) {
    std::ostringstream oss;
    oss << "Fewer XYZ particles were found in the RMF file (" << v.numxyz
        << ") than were provided (" << ncoord << ") in the numpy array";
    throw std::domain_error(oss.str());
  }
}
%}
#endif
