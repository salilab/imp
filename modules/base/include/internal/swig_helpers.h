/**
 *  \file internal/swig_helpers.h
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_SWIG_HELPERS_H
#define IMPBASE_INTERNAL_SWIG_HELPERS_H

// Python.h must be included first
#include <IMP/base/base_config.h>
#include "../base_macros.h"
#include "../Object.h"
#include "../types.h"
#include "../Vector.h"
#include "../Array.h"
#include "../ConstVector.h"
#include "../nullptr_macros.h"
#include "IMP/base/Vector.h"
#include "IMP/base/nullptr.h"
#include <vector>
#include <cstdio>

//using namespace IMP;
using namespace IMP::base;
using namespace IMP;
#ifndef SWIG
  template <bool REFED>
  struct PyPointer: boost::noncopyable {
    PyObject* ptr_;
    PyPointer(PyObject* ptr): ptr_(ptr){
      IMP_INTERNAL_CHECK(ptr, "nullptr pointer passed");
      if (!REFED) {
        Py_INCREF(ptr_);
      } else {
        IMP_INTERNAL_CHECK(ptr_->ob_refcnt >=1, "No refcount");
      }
    }
    operator PyObject*() const{
      return ptr_;
    }
    PyObject* operator->() const {
      return ptr_;
    }
    PyObject *release() {
      IMP_INTERNAL_CHECK(ptr_->ob_refcnt >=1, "No refcount");
      PyObject *ret=ptr_;
      ptr_= IMP_NULLPTR;
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

#define IMP_PYTHON_CALL(call) {int rc=call;                     \
    if (rc != 0) {                                              \
      IMP_INTERNAL_CHECK(0, "Python call failed: "<< #call      \
                         << " with "  << rc);                   \
    }                                                           \
  }

  using boost::enable_if;
  using boost::mpl::and_;
  using boost::mpl::not_;
  using boost::is_convertible;
  //using namespace boost;
  //using namespace boost::mpl;

  template <class V>
  void assign(V* &a, const V &b) {
    a= new V(b);
  }
  template <class V>
  void assign(V &a, const V &b) {
    a= b;
  }
  template <class V>
  void assign(SwigValueWrapper<V> &a, const V &b) {
    a= b;
  }
  template <class T>
  void delete_if_pointer(T&t) {
    t=T();
  }
  template <class T>
  void delete_if_pointer(T*&t) {
    if (t) {
      *t=T();
      delete t;
    }
  }
  template <class T>
  void delete_if_pointer(SwigValueWrapper<T> &v) {
  }


  /*
    Handle assignment into a container. Swig always provides the
    values by reference so we need to determine if the container wants
    them by value or reference and dereference if necessary.
  */
  template <class Container, class Value>
  struct Assign {
    static void assign(Container &c, unsigned int i, Value *v) {
      c[i]=*v;
    }
    static void assign(Container &c, unsigned int i, const Value &v) {
      c[i]=v;
    }

  };


  /*
    Return a reference to a value-type and a pointer to an
    object-type. Also, return what the storage type is (since we get a
    pointer for objects, but just want the object type).
  */

  template <class T, class Enabled=void>
  struct ValueOrObject {
    static const T& get(const T&t) {return t;}
    static const T& get(const T*t) {return *t;}
    typedef T type;
    typedef T store_type;
  };

  template <class T>
  struct ValueOrObject<T,
               typename enable_if<boost::is_base_of<Object, T> >::type > {
    static const T* get(const T*t) {return *t;}
    typedef T type;
    typedef T* store_type;
  };

  template <class T>
  struct ValueOrObject<T*,
               typename enable_if<boost::is_base_of<Object, T> >::type > {
    static const T* get(const T*t) {return *t;}
    typedef T type;
    typedef T* store_type;
  };

  template <class T>
  struct ValueOrObject<Pointer<T>,
               typename enable_if<boost::is_base_of<Object, T> >::type > {
    static const T* get(const T*t) {return *t;}
    typedef T type;
    typedef T* store_type;
  };

  template <class T>
  struct ValueOrObject<WeakPointer<T>,
               typename enable_if<boost::is_base_of<Object, T> >::type > {
    static const T* get(const T*t) {return *t;}
    typedef T type;
    typedef T* store_type;
  };

  template <class T>
  struct ValueOrObject<UncheckedWeakPointer<T>,
               typename enable_if<boost::is_base_of<Object, T> >::type > {
    static const T* get(const T*t) {return *t;}
    typedef T type;
    typedef T* store_type;
  };



  template <class T>
  struct ConvertAllBase {
    BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *o, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      void *vp;
      int res= SWIG_ConvertPtr(o, &vp,st, 0 );
      return SWIG_IsOK(res) &&  vp;
    }
  };


  template <class T>
  struct ConvertValueBase: public ConvertAllBase<T> {
    BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
    BOOST_STATIC_ASSERT(!(boost::is_base_of< Object, T>::value));
    template <class SwigData>
    static const T& get_cpp_object(PyObject *o,
                                   SwigData st, SwigData, SwigData) {
      void *vp;
      int res=SWIG_ConvertPtr(o, &vp,st, 0 );
      if (!SWIG_IsOK(res)) {
        IMP_THROW( "Wrong type.", ValueException);
      }
      if (!vp) {
        IMP_THROW( "Wrong type.", ValueException);
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
  struct ConvertObjectBase: public ConvertAllBase<T> {
    BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
    BOOST_STATIC_ASSERT((boost::is_base_of< Object, T>::value)
                        || (boost::is_same<Object, T>::value));
    template <class SwigData>
    static T* get_cpp_object(PyObject *o, SwigData st, SwigData, SwigData) {
      void *vp;
      int res=SWIG_ConvertPtr(o, &vp,st, 0 );
      if (!SWIG_IsOK(res)) {
        IMP_THROW( "Wrong type.", ValueException);
      }
      if (!vp) {
        IMP_THROW( "Wrong type.", ValueException);
      }
      T* p= reinterpret_cast<T*>(vp);
      IMP_CHECK_OBJECT(static_cast<Object*>(p));
      return p;
    }
    template <class SwigData>
    static PyObject* create_python_object( T* t, SwigData st, int OWN) {
      IMP_CHECK_OBJECT(t);
      PyReceivePointer o(SWIG_NewPointerObj(t, st, OWN));
      IMP::base::internal::ref(t);
      return o.release();
    }
  };

  // T should not be a pointer to the object
  template <class T>
  struct ConvertRAII: public ConvertAllBase<T> {
    BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
    template <class SwigData>
    static T* get_cpp_object(PyObject *o, SwigData st, SwigData, SwigData) {
      void *vp;
      int res=SWIG_ConvertPtr(o, &vp,st, 0 );
      if (!SWIG_IsOK(res)) {
        IMP_THROW( "Wrong type.", ValueException);
      }
      if (!vp) {
        IMP_THROW( "Wrong type.", ValueException);
      }
      T* p= reinterpret_cast<T*>(vp);
      return p;
    }
    template <class SwigData>
    static PyObject* create_python_object( T* t, SwigData st, int OWN) {
      PyReceivePointer o(SWIG_NewPointerObj(t, st, OWN));
      IMP::base::internal::ref(t);
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
    from Decorator to Particle* (so their swig type info is passed around
    even when it is not needed).
  */
  template <class T, class Enabled=void>
  struct Convert: public ConvertValueBase<T > {
    static const int converter=0;
  };


  template <class T>
  struct Convert<T, typename enable_if< boost::is_base_of< Object, T> >
                 ::type >: public ConvertObjectBase<T > {
    static const int converter=1;
  };

  /* Older Boosts return false for is_base_of<Object,Object> so provide
     a specialization for Object */
  template <>
  struct Convert<Object> : public ConvertObjectBase<Object> {
    static const int converter=1;
  };

  template <class T>
  struct Convert<T*, typename enable_if< boost::is_base_of< Object, T> >
                 ::type >: public ConvertObjectBase<T > {
    static const int converter=1;
  };



  /*
    Sequences are anything which gets converted to/from a python sequence.
    These all result in more than one layer of python objects being created
    instead of a single one as in the above cases.
  */
  template <class T, class VT, class ConvertVT>
  struct ConvertSequenceHelper {
    typedef typename ValueOrObject< VT >::type V;
    BOOST_STATIC_ASSERT(!boost::is_pointer<T>::value);
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *in, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      if (!in || !PySequence_Check(in)) {
        return false;
      }
      for (unsigned int i=0; i< PySequence_Length(in); ++i) {
        PyReceivePointer o(PySequence_GetItem(in,i));
        if(! ConvertVT::get_is_cpp_object(o, st, particle_st,
                                           decorator_st)) {
          return false;
        }
      }
      return true;
    }
    template <class SwigData, class C>
    static void fill(PyObject *in, SwigData st,
                     SwigData particle_st, SwigData decorator_st, C&t) {
      if (!in || !PySequence_Check(in)) {
        PyErr_SetString(PyExc_ValueError,"Expected a sequence");
      }
      unsigned int l= PySequence_Size(in);
      IMP_INTERNAL_CHECK(in->ob_refcnt >0, "Freed sequence object found");
      for (unsigned int i=0; i< l; ++i) {
        PyReceivePointer o(PySequence_GetItem(in,i));
        typename ValueOrObject<V>::store_type vs
          =ConvertVT::get_cpp_object(o,st,
                                      particle_st, decorator_st);
        Assign<C, VT>::assign(t, i, vs);
      }
    }
  };

/* describe how to translate between a python sequence and the C++ type
 */

  template <class T, class ConvertValue, class Enabled=void>
  struct ConvertSequence {

  };

  // use an array as an intermediate since pair is not a sequence
  template <class T, class ConvertT>
  struct ConvertSequence<std::pair<T,T>, ConvertT > {
    static const int converter=6;
    typedef boost::array<T,2> Intermediate;
    typedef ConvertSequenceHelper<Intermediate, T, ConvertT > Helper;
    typedef typename ValueOrObject< T >::type VT;
    template <class SwigData>
    static std::pair<T,T> get_cpp_object(PyObject *o,
                                         SwigData st,
                                         SwigData particle_st,
                                         SwigData decorator_st) {
      if (!get_is_cpp_object(o, st, particle_st, decorator_st)) {
        IMP_THROW("Argument not of correct type", ValueException);
      }
      Intermediate im;
      Helper::fill(o, st, particle_st, decorator_st, im);
      std::pair<T,T> ret;
      ret.first=im[0];
      ret.second=im[1];
      return ret;
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *in, SwigData st,
                           SwigData particle_st, SwigData decorator_st) {
      if (!Helper::get_is_cpp_object(in, st, particle_st,
                                     decorator_st)) return false;
      else return PySequence_Size(in) == 2;
    }
    template <class SwigData>
    static PyObject* create_python_object(const std::pair<T,T>& t,
                                          SwigData st, int OWN) {
      PyReceivePointer ret(PyTuple_New(2));
      PyReceivePointer of(Convert<VT>::create_python_object(t.first, st, OWN));
      IMP_PYTHON_CALL(PyTuple_SetItem(ret,0,of.release()));
      PyReceivePointer os(Convert<VT>::create_python_object(t.second, st, OWN));
      IMP_PYTHON_CALL(PyTuple_SetItem(ret,1,os.release()));
      return ret.release();
    }
  };


  template <class T, class ConvertT>
  struct ConvertVectorBase {
    typedef ConvertSequenceHelper<T, typename T::value_type, ConvertT> Helper;
    typedef typename ValueOrObject< typename T::value_type >::type VT;
    template <class SwigData>
    static T get_cpp_object(PyObject *o, SwigData st,
                            SwigData particle_st, SwigData decorator_st) {
      if (!get_is_cpp_object(o, st, particle_st, decorator_st)) {
        IMP_THROW("Argument not of correct type", ValueException);
      }
      T ret(PySequence_Size(o));
      Helper::fill(o, st, particle_st, decorator_st, ret);
      return ret;
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *in, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      return Helper::get_is_cpp_object(in, st, particle_st, decorator_st);
    }
    template <class SwigData>
    static PyObject* create_python_object(const T& t, SwigData st, int OWN) {
      PyReceivePointer ret(PyList_New(t.size()));
      for (unsigned int i=0; i< t.size(); ++i) {
        PyReceivePointer o(ConvertT::create_python_object(t[i], st, OWN));
        // this does not increment the ref count
        IMP_PYTHON_CALL(PyList_SetItem(ret, i, o.release()));
      }
      return ret.release();
    }
  };

  template <class T, class ConvertT>
  struct ConvertSequence<base::Vector<T>, ConvertT > :
    public ConvertVectorBase<base::Vector<T>, ConvertT> {
    static const int converter=7;
  };

template <unsigned int D, class T, class TS, class ConvertT>
struct ConvertSequence<base::Array<D, T, TS>, ConvertT > {
    typedef ConvertSequenceHelper<T, TS, ConvertT> Helper;
    typedef TS VT;
    template <class SwigData>
    static base::Array<D, T, TS> get_cpp_object(PyObject *o, SwigData st,
                            SwigData particle_st, SwigData decorator_st) {
      if (!get_is_cpp_object(o, st, particle_st, decorator_st)) {
        IMP_THROW("Argument not of correct type", ValueException);
      }
      if (PySequence_Size(o) != D) {
        IMP_THROW("Expected tuple of size " << D
                  << " but got one of size " << PySequence_Size(o),
                  ValueException);
      }
      base::Array<D, T, TS> ret;
      Helper::fill(o, st, particle_st, decorator_st, ret);
      return ret;
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *in, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      return Helper::get_is_cpp_object(in, st, particle_st, decorator_st);
    }
    template <class SwigData>
    static PyObject* create_python_object(const base::Array<D, T, TS>& t,
                                          SwigData st, int OWN) {
      PyReceivePointer ret(PyTuple_New(D));
      for (unsigned int i=0; i< t.size(); ++i) {
        PyReceivePointer o(ConvertT::create_python_object(t[i], st, OWN));
        // this does not increment the ref count
        IMP_PYTHON_CALL(PyTuple_SetItem(ret, i, o.release()));
      }
      return ret.release();
    }
    static const int converter=30;
  };

template <class T, class TS, class ConvertT>
struct ConvertSequence<base::ConstVector<T, TS>, ConvertT > :
  public ConvertVectorBase<base::ConstVector<T, TS>, ConvertT> {
    static const int converter=31;
  };

  template <>
  struct Convert<std::string> {
    static const int converter=10;
    template <class SwigData>
    static std::string get_cpp_object(PyObject *o, SwigData st,
                                      SwigData particle_st,
                                      SwigData decorator_st) {
      if (!o || !PyString_Check(o)) {
        IMP_THROW("Not all objects in list have correct type.",
                  ValueException);
      } else {
        return std::string(PyString_AsString(o));
      }
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *o, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      return PyString_Check(o);
    }
    template <class SwigData>
    static PyObject* create_python_object(std::string f, SwigData st, int OWN) {
      return PyString_FromString(f.c_str());
    }
  };

  struct ConvertFloatBase {
    template <class SwigData>
    static double get_cpp_object(PyObject *o, SwigData st,
                                 SwigData particle_st, SwigData decorator_st) {
      if (!o || !PyNumber_Check(o)) {
        IMP_THROW("Not all objects in list have correct type.",
                  ValueException);
      } else {
        return PyFloat_AsDouble(o);
      }
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *o, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      return PyNumber_Check(o);
    }
    template <class SwigData>
    static PyObject* create_python_object(float f, SwigData st, int OWN) {
      // these may or may not have a refcount
      return PyFloat_FromDouble(f);
    }
  };





  template <>
  struct Convert<float>: public ConvertFloatBase {
    static const int converter=11;
  };
  template <>
  struct Convert<double>: public ConvertFloatBase {
    static const int converter=12;
  };

/* with swig 2.0.6 we seem to need both the Int and Long checks */
  template <>
  struct Convert<int> {
    static const int converter=13;
    template <class SwigData>
    static int get_cpp_object(PyObject *o, SwigData st,
                              SwigData particle_st, SwigData decorator_st) {
      if (PyInt_Check(o)) {
        return PyInt_AsLong(o);
      } else if (PyLong_Check(o)) {
        return PyLong_AsLong(o);
      } else {
        IMP_THROW("Not all objects in list have correct number type.",
                  ValueException);
      }
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *o, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      return PyLong_Check(o) || PyInt_Check(o);
    }
    template <class SwigData>
    static PyObject* create_python_object(int f, SwigData st, int OWN) {
      // These may or may not have a ref count
      return PyInt_FromLong(f);
    }
  };



#endif


#endif  /* IMPBASE_INTERNAL_SWIG_HELPERS_H */
