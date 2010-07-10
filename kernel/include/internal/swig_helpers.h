/**
 *  \file internal/swig_helpers.h
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMP_INTERNAL_SWIG_HELPERS_H
#define IMP_INTERNAL_SWIG_HELPERS_H

// Python.h must be included first
#include "../kernel_config.h"
#include "../Particle.h"
#include "../ParticleTuple.h"
#include "../Restraint.h"
#include "../SingletonScore.h"
#include "../macros.h"
#include "../VectorOfRefCounted.h"
#include <boost/array.hpp>
#include <vector>

IMP_BEGIN_INTERNAL_NAMESPACE
namespace swig {
#ifndef SWIG
  using boost::enable_if;
  using boost::mpl::and_;
  using boost::mpl::not_;
  using boost::is_convertible;
  using boost::is_base_of;
  using boost::is_pointer;
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
  struct ValueOrObject<T, typename enable_if<is_base_of<Object, T> >::type > {
    static const T* get(const T*t) {return *t;}
    typedef T type;
    typedef T* store_type;
  };

  template <class T>
  struct ValueOrObject<T*, typename enable_if<is_base_of<Object, T> >::type > {
    static const T* get(const T*t) {return *t;}
    typedef T type;
    typedef T* store_type;
  };



  template <class T>
  struct ConvertAllBase {
    BOOST_STATIC_ASSERT(!is_pointer<T>::value);
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
    BOOST_STATIC_ASSERT(!is_pointer<T>::value);
    BOOST_STATIC_ASSERT(!(is_base_of< Object, T>::value));
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
      PyObject *o = SWIG_NewPointerObj(new T(t), st, OWN);
      return o;
    }
  };


  // T should not be a pointer to the object
  template <class T>
  struct ConvertObjectBase: public ConvertAllBase<T> {
    BOOST_STATIC_ASSERT(!is_pointer<T>::value);
    BOOST_STATIC_ASSERT((is_base_of< Object, T>::value));
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
      PyObject *o = SWIG_NewPointerObj(t, st, OWN);
      internal::ref(t);
      return o;
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
  struct Convert<T, typename enable_if<and_<is_base_of< Object, T>,
                                            not_< is_base_of<Particle, T> > > >
                 ::type >: public ConvertObjectBase<T > {
    static const int converter=1;
  };

  template<>
  struct Convert<Particle >:
    public ConvertObjectBase<Particle> {
    static const int converter=2;
    template <class SwigData>
    static Particle* get_cpp_object(PyObject *o, SwigData st,
                             SwigData particle_st, SwigData decorator_st) {
      void *vp;
      int res=SWIG_ConvertPtr(o, &vp,particle_st, 0 );
      Particle *p=NULL;
      if (!SWIG_IsOK(res)) {
        int res=SWIG_ConvertPtr(o, &vp,decorator_st, 0 );
        if (!SWIG_IsOK(res)) {
          IMP_THROW( "Not all objects in list have correct object type.",
                     ValueException);
        } else {
          Decorator* d= reinterpret_cast<Decorator*>(vp);
          if (*d) {
            p= d->get_particle();
          } else {
            p=NULL;
          }
        }
      } else {
        p= reinterpret_cast<Particle*>(vp);
      }
      if (p) {
        IMP_CHECK_OBJECT(static_cast<Object*>(p));
      }
      return p;
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *o, SwigData st,
                                  SwigData particle_st,
                                  SwigData decorator_st) {
      try {
        get_cpp_object(o, st, particle_st, decorator_st);
      } catch (...) {
        return 0;
      }
      return 1;
    }
  };

  template <class T>
  struct Convert<T,
                 typename enable_if< and_< is_base_of<Decorator, T>,
                             not_<typename T::DecoratorHasTraits > > >::type >:
    public ConvertValueBase<T> {
    static const int converter=3;
    template <class SwigData>
    static T get_cpp_object(PyObject *o, SwigData st,
                            SwigData particle_st, SwigData decorator_st) {
      Particle *p= Convert<Particle>::get_cpp_object(o, particle_st,
                                                     particle_st, decorator_st);
      if (!T::particle_is_instance(p)) {
        IMP_THROW( "Not all objects in list have correct object type: "
                   << p->get_name(),
                   ValueException);
      }
      return T(p);
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *o, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      try {
        get_cpp_object(o, st, particle_st, decorator_st);
      } catch (...) {
        return 0;
      }
      return 1;
    }
  };

  template <class T>
  struct Convert<T, typename enable_if<typename T::DecoratorHasTraits >::type >:
    public ConvertValueBase<T> {
    static const int converter=4;
    template <class SwigData>
    static T get_cpp_object(PyObject *o, SwigData st,
                            SwigData particle_st, SwigData decorator_st) {
      try {
        return ConvertValueBase<T>::get_cpp_object(o, st,
                                                   particle_st, decorator_st);
      } catch (ValueException) {
        Particle *p= Convert<Particle>::get_cpp_object(o, particle_st,
                                                     particle_st,
                                                       decorator_st);
        if (!T::particle_is_instance(p)) {
          IMP_THROW( "Not all objects in list have correct object type: "
                     << p->get_name(),
                     ValueException);
        }
        return T(p);
      }
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *o, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      try {
        get_cpp_object(o, st, particle_st, decorator_st);
      } catch (...) {
        return 0;
      }
      return 1;
    }
  };

  /*
    Sequences are anything which gets converted to/from a python sequence.
    These all result in more than one layer of python objects being created
    instead of a single one as in the above cases.
  */
  template <class T, class VT>
  struct ConvertSequenceHelper {
    typedef typename ValueOrObject< VT >::type V;
    BOOST_STATIC_ASSERT(!is_pointer<T>::value);
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *in, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      if (!in || !PySequence_Check(in)) {
        return false;
      }
      for (unsigned int i=0; i< PySequence_Length(in); ++i) {
        PyObject *o = PySequence_GetItem(in,i);
        if(! Convert<V>::get_is_cpp_object(o, st, particle_st,
                                           decorator_st)) return false;
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
      for (unsigned int i=0; i< l; ++i) {
        PyObject *o = PySequence_GetItem(in,i);
        typename ValueOrObject<V>::store_type vs
          =Convert<V>::get_cpp_object(o,st,
                                      particle_st, decorator_st);
        Assign<C, VT>::assign(t, i, vs);
        Py_DECREF(o);
      }
    }
  };

  template <class T>
  struct Convert<T, typename enable_if< is_base_of<
       boost::array<typename T::value_type, T::static_size>, T> >::type > {
    static const int converter=5;
    typedef ConvertSequenceHelper<T, typename T::value_type> Helper;
    typedef typename ValueOrObject< typename T::value_type >::type VT;
    template <class SwigData>
    static T get_cpp_object(PyObject *o, SwigData st,
                            SwigData particle_st, SwigData decorator_st) {
      if (!get_is_cpp_object(o, st, particle_st, decorator_st)) {
        IMP_THROW("Argument not of correct type", ValueException);
      }
      T ret;
      Helper::fill(o, st, particle_st, decorator_st, ret);
      return ret;
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *in, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      if (!Helper::get_is_cpp_object(in, st,
                                     particle_st, decorator_st)) return false;
      else return PySequence_Size(in) ==T::static_size;
    }
    template <class SwigData>
    static PyObject* create_python_object(const T& t, SwigData st, int OWN) {
      PyObject *ret= PyTuple_New(T::static_size);
      for (unsigned int i=0; i< T::static_size; ++i) {
        PyObject *o = Convert<VT>::create_python_object(t[i], st, OWN);
        PyTuple_SetItem(ret,i,o);
        //Py_DECREF(o);
      }
      return ret;
    }
  };


  // use an array as an intermediate since pair is not a sequence
  template <class T>
  struct Convert<std::pair<T,T> > {
    static const int converter=6;
    typedef boost::array<T,2> Intermediate;
    typedef ConvertSequenceHelper<Intermediate, T> Helper;
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
      PyObject *ret= PyTuple_New(2);
      PyObject *of = Convert<VT>::create_python_object(t.first, st, OWN);
      PyTuple_SetItem(ret,0,of);
      //Py_DECREF(of);
      PyObject *os = Convert<VT>::create_python_object(t.second, st, OWN);
      PyTuple_SetItem(ret,1,os);
      //Py_DECREF(os);
      return ret;
    }
  };


  template <class T>
  struct ConvertVectorBase {
    typedef ConvertSequenceHelper<T, typename T::value_type> Helper;
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
      PyObject *ret= PyList_New(t.size());
      for (unsigned int i=0; i< t.size(); ++i) {
        PyObject *o = Convert<VT>::create_python_object(t[i], st, OWN);
        // this doesn't seem to increment the ref count for some reason
        PyList_SetItem(ret, i, o);
        //Py_DECREF(o);
      }
      return ret;
    }
  };



  template <class T>
  struct Convert<std::vector<T> > :
    public ConvertVectorBase<std::vector<T> > {
    static const int converter=7;
  };

  template <class T, class P>
  struct Convert<VectorOfRefCounted<T, P> > :
    public ConvertVectorBase< VectorOfRefCounted<T, P> > {
    static const int converter=8;
  };

  template <class T, class P>
  struct Convert<Decorators<T, P> > :
    public ConvertVectorBase< Decorators<T, P> > {
    static const int converter=9;
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
      PyObject *o = PyString_FromString(f.c_str());
      Py_INCREF(o);
      return o;
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
      PyObject *o = PyFloat_FromDouble(f);
      Py_INCREF(o);
      return o;
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

  template <>
  struct Convert<int> {
    static const int converter=13;
    template <class SwigData>
    static int get_cpp_object(PyObject *o, SwigData st,
                              SwigData particle_st, SwigData decorator_st) {
      if (!PyInt_Check(o)) {
        IMP_THROW("Not all objects in list have correct number type.",
                  ValueException);
      } else {
        return PyInt_AsLong(o);
      }
    }
    template <class SwigData>
    static bool get_is_cpp_object(PyObject *o, SwigData st,
                                  SwigData particle_st, SwigData decorator_st) {
      return PyInt_Check(o);
    }
    template <class SwigData>
    static PyObject* create_python_object(int f, SwigData st, int OWN) {
      PyObject *o = PyInt_FromLong(f);
      Py_INCREF(o);
      return o;
    }
  };

#endif

}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_SWIG_HELPERS_H */
