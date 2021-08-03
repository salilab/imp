/**
 *  \file internal/swig_helpers.h
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_SWIG_HELPERS_BASE_H
#define IMPKERNEL_INTERNAL_SWIG_HELPERS_BASE_H

// Python.h must be included first
#include <IMP/kernel_config.h>
#include "../base_macros.h"
#include "../Object.h"
#include "../types.h"
#include "../Vector.h"
#include "../Array.h"
#include "../ConstVector.h"
#include "../nullptr_macros.h"
#include "IMP/Vector.h"
#include "IMP/nullptr.h"
#include <vector>
#include <cstdio>

#if IMP_KERNEL_HAS_NUMPY
#include <numpy/arrayobject.h>
// This should be defined in the including module's SWIG wrapper
extern int numpy_import_retval;

// Return true iff `o` is a numpy array of the given type laid out in
// a contiguous chunk of memory
bool is_native_numpy_array(PyObject *o, int numpy_type) {
  if (!o || !PyArray_Check(o)) return false; // not a numpy array

  PyArrayObject *a = (PyArrayObject *)o;
  int array_type = PyArray_TYPE(a);
  if (array_type != NPY_NOTYPE
      && !PyArray_EquivTypenums(array_type, numpy_type)) {
    return false;  // data type does not match
  }

  return PyArray_ISCONTIGUOUS(a) && PyArray_ISNOTSWAPPED(a);
}

// Return true iff `o` is a 1D numpy array of the given type laid out in
// a contiguous chunk of memory
bool is_native_numpy_1d_array(PyObject *o, int numpy_type) {
  if (is_native_numpy_array(o, numpy_type)) {
    PyArrayObject *a = (PyArrayObject *)o;
    return PyArray_NDIM(a) == 1;
  } else {
    return false;
  }
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
#endif

// using namespace IMP;
using namespace IMP;
#ifndef SWIG
template <bool REFED>
struct PyPointer : boost::noncopyable {
  PyObject* ptr_;
  PyPointer(PyObject* ptr) : ptr_(ptr) {
    IMP_INTERNAL_CHECK(ptr, "nullptr pointer passed");
    if (!REFED) {
      Py_INCREF(ptr_);
    } else {
      IMP_INTERNAL_CHECK(ptr_->ob_refcnt >= 1, "No refcount");
    }
  }
  operator PyObject*() const { return ptr_; }
  PyObject* operator->() const { return ptr_; }
  PyObject* release() {
    IMP_INTERNAL_CHECK(ptr_->ob_refcnt >= 1, "No refcount");
    PyObject* ret = ptr_;
    ptr_ = IMP_NULLPTR;
    return ret;
  }
  ~PyPointer() {
    if (ptr_) {
      Py_DECREF(ptr_);
    }
  }
};
typedef PyPointer<true> PyReceivePointer;
typedef PyPointer<false> PyOwnerPointer;

#define IMP_PYTHON_CALL(call)                                           \
  {                                                                     \
    int rc = call;                                                      \
    if (rc != 0) {                                                      \
      IMP_INTERNAL_CHECK(0, "Python call failed: " << #call << " with " \
                                                   << rc);              \
    }                                                                   \
  }

using boost::enable_if;
using boost::mpl::and_;
using boost::mpl::not_;
using boost::is_convertible;
// using namespace boost;
// using namespace boost::mpl;

template <class V>
void assign(V*& a, const V& b) {
  a = new V(b);
}
template <class V>
void assign(V& a, const V& b) {
  a = b;
}
template <class V>
void assign(SwigValueWrapper<V>& a, const V& b) {
  a = b;
}
template <class T>
void delete_if_pointer(T& t) {
  t = T();
}
template <class T>
void delete_if_pointer(T*& t) {
  if (t) {
    *t = T();
    delete t;
  }
}
template <class T>
void delete_if_pointer(SwigValueWrapper<T>&) {}

/*
  Handle assignment into a container. Swig always provides the
  values by reference so we need to determine if the container wants
  them by value or reference and dereference if necessary.
*/
template <class Container, class Value>
struct Assign {
  static void assign(Container& c, unsigned int i, Value* v) { c[i] = *v; }
  static void assign(Container& c, unsigned int i, const Value& v) { c[i] = v; }
};

/*
  Return a reference to a value-type and a pointer to an
  object-type. Also, return what the storage type is (since we get a
  pointer for objects, but just want the object type).
*/

template <class T, class Enabled = void>
struct ValueOrObject {
  static const T& get(const T& t) { return t; }
  static const T& get(const T* t) { return *t; }
  typedef T type;
  typedef T store_type;
};

template <class T>
struct ValueOrObject<T,
                     typename enable_if<boost::is_base_of<Object, T> >::type> {
  static const T* get(const T* t) { return *t; }
  typedef T type;
  typedef T* store_type;
};

template <class T>
struct ValueOrObject<T*,
                     typename enable_if<boost::is_base_of<Object, T> >::type> {
  static const T* get(const T* t) { return *t; }
  typedef T type;
  typedef T* store_type;
};

template <class T>
struct ValueOrObject<Pointer<T>,
                     typename enable_if<boost::is_base_of<Object, T> >::type> {
  static const T* get(const T* t) { return *t; }
  typedef T type;
  typedef T* store_type;
};

template <class T>
struct ValueOrObject<WeakPointer<T>,
                     typename enable_if<boost::is_base_of<Object, T> >::type> {
  static const T* get(const T* t) { return *t; }
  typedef T type;
  typedef T* store_type;
};

template <class T>
struct ValueOrObject<UncheckedWeakPointer<T>,
                     typename enable_if<boost::is_base_of<Object, T> >::type> {
  static const T* get(const T* t) { return *t; }
  typedef T type;
  typedef T* store_type;
};

inline std::string get_convert_error(const char *err, const char *symname,
                                     int argnum, const char *argtype) {
  std::ostringstream msg;
  msg << err << " in '" << symname << "', argument " << argnum
      << " of type '" << argtype << "'";
  return msg.str();
}

template <class T>
struct ConvertAllBase {
  BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* o, SwigData st, SwigData, SwigData) {
    void* vp;
    int res = SWIG_ConvertPtr(o, &vp, st, 0);
    return SWIG_IsOK(res) && vp;
  }
};

template <class T>
struct ConvertValueBase : public ConvertAllBase<T> {
  BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
  BOOST_STATIC_ASSERT(!(boost::is_base_of<Object, T>::value));
  template <class SwigData>
  static const T& get_cpp_object(PyObject* o, const char *symname, int argnum,
                                 const char *argtype, SwigData st, SwigData,
                                 SwigData) {
    void* vp;
    int res = SWIG_ConvertPtr(o, &vp, st, 0);
    if (!SWIG_IsOK(res)) {
      IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                TypeException);
    }
    if (!vp) {
      IMP_THROW(get_convert_error("NULL value", symname, argnum, argtype),
                ValueException);
    }
    return *reinterpret_cast<T*>(vp);
  }
  template <class SwigData>
  static PyObject* create_python_object(T t, SwigData st, int OWN) {
    PyReceivePointer o(SWIG_NewPointerObj(new T(t), st, OWN));
    return o.release();
  }
};

// T should not be a pointer to the object
template <class T>
struct ConvertObjectBase : public ConvertAllBase<T> {
  BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
  BOOST_STATIC_ASSERT((boost::is_base_of<Object, T>::value) ||
                      (boost::is_same<Object, T>::value));
  template <class SwigData>
  static T* get_cpp_object(PyObject* o, const char *symname, int argnum,
                           const char *argtype, SwigData st, SwigData,
                           SwigData) {
    void* vp;
    int res = SWIG_ConvertPtr(o, &vp, st, 0);
    if (!SWIG_IsOK(res)) {
      IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                TypeException);
    }
    if (!vp) {
      IMP_THROW(get_convert_error("NULL value", symname, argnum, argtype),
                ValueException);
    }
    T* p = reinterpret_cast<T*>(vp);
    IMP_CHECK_OBJECT(static_cast<Object*>(p));
    return p;
  }
  template <class SwigData>
  static PyObject* create_python_object(T* t, SwigData st, int OWN) {
    IMP_CHECK_OBJECT(t);
    PyReceivePointer o(SWIG_NewPointerObj(t, st, OWN));
    t->ref();
    return o.release();
  }
};

// T should not be a pointer to the object
template <class T>
struct ConvertRAII : public ConvertAllBase<T> {
  BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
  template <class SwigData>
  static T* get_cpp_object(PyObject* o, const char *symname, int argnum,
                           const char *argtype, SwigData st, SwigData,
                           SwigData) {
    void* vp;
    int res = SWIG_ConvertPtr(o, &vp, st, 0);
    if (!SWIG_IsOK(res)) {
      IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                TypeException);
    }
    if (!vp) {
      IMP_THROW(get_convert_error("NULL value", symname, argnum, argtype),
                ValueException);
    }
    T* p = reinterpret_cast<T*>(vp);
    return p;
  }
  template <class SwigData>
  static PyObject* create_python_object(T* t, SwigData st, int OWN) {
    PyReceivePointer o(SWIG_NewPointerObj(t, st, OWN));
    t->ref();
    return o.release();
  }
};

/*
    Provide support for converting from python objects to the appropriate
    C++ object.
    - get_cpp_object takes a python object and returns a pointer to the
    needed C++ object
    - get_is_cpp_object checks if the python object can be converted to
    the needed C++ object
    - create_python_object takes the C++ object and creates a python object

    Decorators are a special case in order to provide the implicit conversion
    from Decorator to Particle* (so their swig type info is passed
    around
    even when it is not needed).
  */
template <class T, class Enabled = void>
struct Convert : public ConvertValueBase<T> {
  static const int converter = 0;
};

template <class T>
struct Convert<T, typename enable_if<boost::is_base_of<
                      Object, T> >::type> : public ConvertObjectBase<T> {
  static const int converter = 1;
};

/* Older Boosts return false for is_base_of<Object,Object> so provide
   a specialization for Object */
template <>
struct Convert<Object> : public ConvertObjectBase<Object> {
  static const int converter = 1;
};

template <class T>
struct Convert<T*, typename enable_if<boost::is_base_of<
                       Object, T> >::type> : public ConvertObjectBase<T> {
  static const int converter = 1;
};

/*
  Sequences are anything which gets converted to/from a python sequence.
  These all result in more than one layer of python objects being created
  instead of a single one as in the above cases.
*/
template <class T, class VT, class ConvertVT>
struct ConvertSequenceHelper {
  typedef typename ValueOrObject<VT>::type V;
  BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    if (!in || !PySequence_Check(in)) {
      return false;
    }
    for (unsigned int i = 0; i < PySequence_Length(in); ++i) {
      PyReceivePointer o(PySequence_GetItem(in, i));
      if (!ConvertVT::get_is_cpp_object(o, st, particle_st, decorator_st)) {
        return false;
      }
    }
    return true;
  }
  template <class SwigData, class C>
  static void fill(PyObject* in, const char *symname, int argnum,
                   const char *argtype, SwigData st, SwigData particle_st,
                   SwigData decorator_st, C& t) {
    if (!in || !PySequence_Check(in)) {
      PyErr_SetString(PyExc_ValueError, "Expected a sequence");
    }
    unsigned int l = PySequence_Size(in);
    IMP_INTERNAL_CHECK(in->ob_refcnt > 0, "Freed sequence object found");
    for (unsigned int i = 0; i < l; ++i) {
      PyReceivePointer o(PySequence_GetItem(in, i));
      typename ValueOrObject<V>::store_type vs =
          ConvertVT::get_cpp_object(o, symname, argnum, argtype, st,
                                    particle_st, decorator_st);
      Assign<C, VT>::assign(t, i, vs);
    }
  }
};

/* describe how to translate between a python sequence and the C++ type
 */

template <class T, class ConvertValue, class Enabled = void>
struct ConvertSequence {};

// use an array as an intermediate since pair is not a sequence
template <class T, class ConvertT>
struct ConvertSequence<std::pair<T, T>, ConvertT> {
  static const int converter = 6;
  typedef boost::array<T, 2> Intermediate;
  typedef ConvertSequenceHelper<Intermediate, T, ConvertT> Helper;
  typedef typename ValueOrObject<T>::type VT;
  template <class SwigData>
  static std::pair<T, T> get_cpp_object(PyObject* o, const char *symname,
                                        int argnum, const char *argtype,
                                        SwigData st,
                                        SwigData particle_st,
                                        SwigData decorator_st) {
    if (!get_is_cpp_object(o, st, particle_st, decorator_st)) {
      IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                TypeException);
    }
    Intermediate im;
    Helper::fill(o, symname, argnum, argtype, st, particle_st,
                 decorator_st, im);
    std::pair<T, T> ret;
    ret.first = im[0];
    ret.second = im[1];
    return ret;
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    if (!Helper::get_is_cpp_object(in, st, particle_st, decorator_st))
      return false;
    else
      return PySequence_Size(in) == 2;
  }
  template <class SwigData>
  static PyObject* create_python_object(const std::pair<T, T>& t, SwigData st,
                                        int OWN) {
    PyReceivePointer ret(PyTuple_New(2));
    PyReceivePointer of(Convert<VT>::create_python_object(t.first, st, OWN));
    IMP_PYTHON_CALL(PyTuple_SetItem(ret, 0, of.release()));
    PyReceivePointer os(Convert<VT>::create_python_object(t.second, st, OWN));
    IMP_PYTHON_CALL(PyTuple_SetItem(ret, 1, os.release()));
    return ret.release();
  }
};

template <class T, class ConvertT>
struct ConvertVectorBase {
  typedef ConvertSequenceHelper<T, typename T::value_type, ConvertT> Helper;
  typedef typename ValueOrObject<typename T::value_type>::type VT;
  template <class SwigData>
  static T get_cpp_object(PyObject* o, const char *symname, int argnum,
                          const char *argtype, SwigData st,
                          SwigData particle_st,
                          SwigData decorator_st) {
    if (!get_is_cpp_object(o, st, particle_st, decorator_st)) {
      IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                TypeException);
    }
    T ret(PySequence_Size(o));
    Helper::fill(o, symname, argnum, argtype, st, particle_st,
                 decorator_st, ret);
    return ret;
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    return Helper::get_is_cpp_object(in, st, particle_st, decorator_st);
  }
  template <class SwigData>
  static PyObject* create_python_object(const T& t, SwigData st, int OWN) {
    PyReceivePointer ret(PyList_New(t.size()));
    for (unsigned int i = 0; i < t.size(); ++i) {
      PyReceivePointer o(ConvertT::create_python_object(t[i], st, OWN));
      // this does not increment the ref count
      IMP_PYTHON_CALL(PyList_SetItem(ret, i, o.release()));
    }
    return ret.release();
  }
};

template <class T, class ConvertT>
struct ConvertSequence<Vector<T>, ConvertT> : public ConvertVectorBase<
                                                        Vector<T>,
                                                        ConvertT> {
  static const int converter = 7;
};

template <unsigned int D, class T, class TS, class ConvertT>
struct ConvertSequence<IMP::Array<D, T, TS>, ConvertT> {
  typedef ConvertSequenceHelper<T, TS, ConvertT> Helper;
  typedef TS VT;
  template <class SwigData>
  static IMP::Array<D, T, TS> get_cpp_object(PyObject* o, const char *symname,
                                             int argnum, const char *argtype,
                                             SwigData st, SwigData particle_st,
                                             SwigData decorator_st) {
    if (!get_is_cpp_object(o, st, particle_st, decorator_st)) {
      IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                TypeException);
    }
    if (PySequence_Size(o) != D) {
      std::ostringstream msg;
      msg << "Expected tuple of size " << D << " but got one of size "
          << PySequence_Size(o);
      IMP_THROW(get_convert_error(msg.str().c_str(), symname, argnum, argtype),
                ValueException);
    }
    IMP::Array<D, T, TS> ret;
    Helper::fill(o, symname, argnum, argtype, st, particle_st,
                 decorator_st, ret);
    return ret;
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    return Helper::get_is_cpp_object(in, st, particle_st, decorator_st);
  }
  template <class SwigData>
  static PyObject* create_python_object(const IMP::Array<D, T, TS>& t,
                                        SwigData st, int OWN) {
    PyReceivePointer ret(PyTuple_New(D));
    for (unsigned int i = 0; i < t.size(); ++i) {
      PyReceivePointer o(ConvertT::create_python_object(t[i], st, OWN));
      // this does not increment the ref count
      IMP_PYTHON_CALL(PyTuple_SetItem(ret, i, o.release()));
    }
    return ret.release();
  }
  static const int converter = 30;
};

template <class T, class TS, class ConvertT>
struct ConvertSequence<
    ConstVector<T, TS>,
    ConvertT> : public ConvertVectorBase<ConstVector<T, TS>, ConvertT> {
  static const int converter = 31;
};

#if IMP_KERNEL_HAS_NUMPY
/* Provide template specializations that copy directly between IMP
   arrays and numpy objects, without having to construct Python objects
   for each sequence element */
template <class ConvertT>
struct ConvertSequence<Ints, ConvertT> : public ConvertVectorBase<
                                                        Ints, ConvertT> {
  static const int converter = 32;
  typedef ConvertVectorBase<Ints, ConvertT> Base;

  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    return (numpy_import_retval == 0 && is_native_numpy_1d_array(in, NPY_INT))
           || Base::get_is_cpp_object(in, st, particle_st, decorator_st);
  }

  template <class SwigData>
  static Ints get_cpp_object(PyObject* o, const char *symname, int argnum,
                             const char *argtype, SwigData st,
                             SwigData particle_st,
                             SwigData decorator_st) {
    if (numpy_import_retval == 0 && is_native_numpy_1d_array(o, NPY_INT)) {
      int dim = PyArray_DIM((PyArrayObject*)o, 0);
      int *data = (int *)PyArray_DATA((PyArrayObject*)o);
      return Ints(data, data+dim);
    } else {
      return Base::get_cpp_object(o, symname, argnum, argtype, st,
                                  particle_st, decorator_st);
    }
  }

  template <class SwigData>
  static PyObject* create_python_object(const Ints& t, SwigData st, int OWN) {
    if (numpy_import_retval == 0) {
      npy_intp dims[2];
      dims[0] = t.size();
      PyReceivePointer ret(PyArray_SimpleNew(1, dims, NPY_INT));
      if (t.size() > 0) {
        PyObject *obj = ret;
        memcpy(PyArray_DATA(obj), &t[0], t.size() * sizeof(int));
      }
      return ret.release();
    } else {
      return Base::create_python_object(t, st, OWN);
    }
  }
};

template <class ConvertT>
struct ConvertSequence<Floats, ConvertT> : public ConvertVectorBase<
                                                        Floats, ConvertT> {
  static const int converter = 33;
  typedef ConvertVectorBase<Floats, ConvertT> Base;

  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    return (numpy_import_retval == 0
                    && is_native_numpy_1d_array(in, NPY_DOUBLE))
           || Base::get_is_cpp_object(in, st, particle_st, decorator_st);
  }

  template <class SwigData>
  static Floats get_cpp_object(PyObject* o, const char *symname, int argnum,
                               const char *argtype, SwigData st,
                               SwigData particle_st,
                               SwigData decorator_st) {
    if (numpy_import_retval == 0 && is_native_numpy_1d_array(o, NPY_DOUBLE)) {
      int dim = PyArray_DIM((PyArrayObject*)o, 0);
      double *data = (double *)PyArray_DATA((PyArrayObject*)o);
      return Floats(data, data+dim);
    } else {
      return Base::get_cpp_object(o, symname, argnum, argtype, st,
                                  particle_st, decorator_st);
    }
  }

  template <class SwigData>
  static PyObject* create_python_object(const Floats& t, SwigData st, int OWN) {
    if (numpy_import_retval == 0) {
      npy_intp dims[2];
      dims[0] = t.size();
      PyReceivePointer ret(PyArray_SimpleNew(1, dims, NPY_DOUBLE));
      if (t.size() > 0) {
        PyObject *obj = ret;
        memcpy(PyArray_DATA(obj), &t[0], t.size() * sizeof(double));
      }
      return ret.release();
    } else {
      return Base::create_python_object(t, st, OWN);
    }
  }
};

/* Map list of ParticleIndex to a numpy array of integers.
   This should be much faster than making a Python ParticleIndex object
   for each index, at the expense of potentially allowing unsafe operations
   on the indexes (e.g. it makes no sense to multiply or divide an index) */
template <class ConvertT>
struct ConvertSequence<ParticleIndexes, ConvertT> : public ConvertVectorBase<
                                                     ParticleIndexes, ConvertT> {
  static const int converter = 34;
  typedef ConvertVectorBase<ParticleIndexes, ConvertT> Base;

  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    return (numpy_import_retval == 0 && is_native_numpy_1d_array(in, NPY_INT))
           || Base::get_is_cpp_object(in, st, particle_st, decorator_st);
  }

  template <class SwigData>
  static ParticleIndexes get_cpp_object(
                             PyObject* o, const char *symname, int argnum,
                             const char *argtype, SwigData st,
                             SwigData particle_st,
                             SwigData decorator_st) {
    if (numpy_import_retval == 0 && is_native_numpy_1d_array(o, NPY_INT)) {
      BOOST_STATIC_ASSERT(sizeof(ParticleIndex) == sizeof(int));
      int dim = PyArray_DIM((PyArrayObject*)o, 0);
      ParticleIndex *data = (ParticleIndex *)PyArray_DATA((PyArrayObject*)o);
      return ParticleIndexes(data, data+dim);
    } else {
      return Base::get_cpp_object(o, symname, argnum, argtype, st,
                                  particle_st, decorator_st);
    }
  }

  template <class SwigData>
  static PyObject* create_python_object(const ParticleIndexes& t, SwigData st,
                                        int OWN) {
    if (numpy_import_retval == 0) {
      BOOST_STATIC_ASSERT(sizeof(ParticleIndex) == sizeof(int));
      npy_intp dims[2];
      dims[0] = t.size();
      PyReceivePointer ret(PyArray_SimpleNew(1, dims, NPY_INT));
      if (t.size() > 0) {
        PyObject *obj = ret;
        memcpy(PyArray_DATA(obj), &t[0], t.size() * sizeof(int));
      }
      return ret.release();
    } else {
      return Base::create_python_object(t, st, OWN);
    }
  }
};

// Convert NumPy array to ParticleIndex{Pairs,Triplets,Quads}
template<class IndexArray, int D>
static IndexArray create_index_array_cpp(PyObject *o) {
  PyArrayObject *a = (PyArrayObject *)o;
  npy_intp sz = PyArray_DIM(a, 0);

  IndexArray arr(sz);
  if (sz > 0) {
    char *data = (char *)PyArray_DATA(o);
    for (size_t i = 0; i < sz; ++i) {
      memcpy(arr[i].data(), data + i * D * sizeof(int), sizeof(int) * D);
    }
  }
  return arr;
}

// Convert ParticleIndex{Pairs,Triplets,Quads} to a NumPy array
template<class IndexArray, int D>
static PyObject* create_index_array_numpy(const IndexArray &t) {
  npy_intp dims[2];
  dims[0] = t.size();
  dims[1] = D;
  PyReceivePointer ret(PyArray_SimpleNew(2, dims, NPY_INT));
  if (t.size() > 0) {
    PyObject *obj = ret;
    char *data = (char *)PyArray_DATA(obj);
    for (size_t i = 0; i < t.size(); ++i) {
      memcpy(data + i * D * sizeof(int), t[i].data(), sizeof(int) * D);
    }
  }
  return ret.release();
}

template <class ConvertT>
struct ConvertSequence<ParticleIndexPairs, ConvertT>
          : public ConvertVectorBase<ParticleIndexPairs, ConvertT> {
  static const int converter = 35;
  typedef ConvertVectorBase<ParticleIndexPairs, ConvertT> Base;

  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    return (numpy_import_retval == 0
                    && is_native_numpy_2d_array(in, NPY_INT, 2))
           || Base::get_is_cpp_object(in, st, particle_st, decorator_st);
  }

  template <class SwigData>
  static ParticleIndexPairs get_cpp_object(
                             PyObject* o, const char *symname, int argnum,
                             const char *argtype, SwigData st,
                             SwigData particle_st,
                             SwigData decorator_st) {
    if (numpy_import_retval == 0 && is_native_numpy_2d_array(o, NPY_INT, 2)) {
      return create_index_array_cpp<ParticleIndexPairs, 2>(o);
    } else {
      return Base::get_cpp_object(o, symname, argnum, argtype, st,
                                  particle_st, decorator_st);
    }
  }

  template <class SwigData>
  static PyObject* create_python_object(const ParticleIndexPairs& t,
                                        SwigData st, int OWN) {
    if (numpy_import_retval == 0) {
      return create_index_array_numpy<ParticleIndexPairs, 2>(t);
    } else {
      return Base::create_python_object(t, st, OWN);
    }
  }
};

template <class ConvertT>
struct ConvertSequence<ParticleIndexTriplets, ConvertT>
          : public ConvertVectorBase<ParticleIndexTriplets, ConvertT> {
  static const int converter = 36;
  typedef ConvertVectorBase<ParticleIndexTriplets, ConvertT> Base;

  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    return (numpy_import_retval == 0
                    && is_native_numpy_2d_array(in, NPY_INT, 3))
           || Base::get_is_cpp_object(in, st, particle_st, decorator_st);
  }

  template <class SwigData>
  static ParticleIndexTriplets get_cpp_object(
                             PyObject* o, const char *symname, int argnum,
                             const char *argtype, SwigData st,
                             SwigData particle_st,
                             SwigData decorator_st) {
    if (numpy_import_retval == 0 && is_native_numpy_2d_array(o, NPY_INT, 3)) {
      return create_index_array_cpp<ParticleIndexTriplets, 3>(o);
    } else {
      return Base::get_cpp_object(o, symname, argnum, argtype, st,
                                  particle_st, decorator_st);
    }
  }

  template <class SwigData>
  static PyObject* create_python_object(const ParticleIndexTriplets& t,
                                        SwigData st, int OWN) {
    if (numpy_import_retval == 0) {
      return create_index_array_numpy<ParticleIndexTriplets, 3>(t);
    } else {
      return Base::create_python_object(t, st, OWN);
    }
  }
};

template <class ConvertT>
struct ConvertSequence<ParticleIndexQuads, ConvertT>
          : public ConvertVectorBase<ParticleIndexQuads, ConvertT> {
  static const int converter = 37;
  typedef ConvertVectorBase<ParticleIndexQuads, ConvertT> Base;

  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    return (numpy_import_retval == 0
                    && is_native_numpy_2d_array(in, NPY_INT, 4))
           || Base::get_is_cpp_object(in, st, particle_st, decorator_st);
  }

  template <class SwigData>
  static ParticleIndexQuads get_cpp_object(
                             PyObject* o, const char *symname, int argnum,
                             const char *argtype, SwigData st,
                             SwigData particle_st,
                             SwigData decorator_st) {
    if (numpy_import_retval == 0 && is_native_numpy_2d_array(o, NPY_INT, 4)) {
      return create_index_array_cpp<ParticleIndexQuads, 4>(o);
    } else {
      return Base::get_cpp_object(o, symname, argnum, argtype, st,
                                  particle_st, decorator_st);
    }
  }

  template <class SwigData>
  static PyObject* create_python_object(const ParticleIndexQuads& t,
                                        SwigData st, int OWN) {
    if (numpy_import_retval == 0) {
      return create_index_array_numpy<ParticleIndexQuads, 4>(t);
    } else {
      return Base::create_python_object(t, st, OWN);
    }
  }
};
#endif

template <>
struct Convert<std::string> {
  static const int converter = 10;
  template <class SwigData>
  static std::string get_cpp_object(PyObject* o, const char *symname,
                                    int argnum, const char *argtype, SwigData,
                                    SwigData, SwigData) {
#if PY_VERSION_HEX>=0x03000000
    if (!o || !PyUnicode_Check(o)) {
      IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                TypeException);
    } else {
      PyObject *obj = PyUnicode_AsUTF8String(o);
      if (!obj) {
        IMP_THROW(get_convert_error("Invalid Unicode", symname, argnum,
                                    argtype),
                  ValueException);
      }
      std::string s(PyString_AsString(obj));
      Py_DECREF(obj);
      return s;
    }
#else
    if (!o || !PyString_Check(o)) {
      IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                TypeException);
    } else {
      return std::string(PyString_AsString(o));
    }
#endif
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* o, SwigData, SwigData, SwigData) {
#if PY_VERSION_HEX>=0x03000000
    return PyUnicode_Check(o);
#else
    return PyString_Check(o);
#endif
  }
  template <class SwigData>
  static PyObject* create_python_object(std::string f, SwigData, int) {
#if PY_VERSION_HEX>=0x03000000
    return PyUnicode_FromString(f.c_str());
#else
    return PyString_FromString(f.c_str());
#endif
  }
};

struct ConvertFloatBase {
  template <class SwigData>
  static double get_cpp_object(PyObject* o, const char *symname, int argnum,
                               const char *argtype, SwigData, SwigData,
                               SwigData) {
    if (!o || !PyNumber_Check(o)) {
      IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                TypeException);
    } else {
      return PyFloat_AsDouble(o);
    }
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* o, SwigData, SwigData, SwigData) {
    return PyNumber_Check(o);
  }
  template <class SwigData>
  static PyObject* create_python_object(double f, SwigData, int) {
    // these may or may not have a refcount
    return PyFloat_FromDouble(f);
  }
};

template <>
struct Convert<float> : public ConvertFloatBase {
  static const int converter = 11;
};
template <>
struct Convert<double> : public ConvertFloatBase {
  static const int converter = 12;
};

/* with swig 2.0.6 we seem to need both the Int and Long checks */
template <>
struct Convert<int> {
  static const int converter = 13;
  template <class SwigData>
  static int get_cpp_object(PyObject* o, const char *symname, int argnum,
                            const char *argtype, SwigData, SwigData, SwigData) {
    if (PyInt_Check(o)) {
      return PyInt_AsLong(o);
    } else if (PyLong_Check(o)) {
      return PyLong_AsLong(o);
    } else {
      long ret = PyLong_AsLong(o);
#if PY_VERSION_HEX < 0x03000000
      if (ret == -1 && PyErr_Occurred()) {
        ret = PyInt_AsLong(o);
      }
#endif
      if (ret != -1 || !PyErr_Occurred()) {
        return ret;
      } else {
        IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                  TypeException);
      }
    }
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* o, SwigData, SwigData, SwigData) {
    return PyLong_Check(o) || PyInt_Check(o) || PyNumber_Check(o);
  }
  template <class SwigData>
  static PyObject* create_python_object(int f, SwigData, int) {
    // These may or may not have a ref count
    return PyInt_FromLong(f);
  }
};

#endif

#endif /* IMPKERNEL_INTERNAL_SWIG_HELPERS_BASE_H */
