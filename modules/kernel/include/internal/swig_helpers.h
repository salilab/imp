/**
 *  \file internal/swig_helpers.h
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_SWIG_HELPERS_H
#define IMPKERNEL_INTERNAL_SWIG_HELPERS_H

// Python.h must be included first
#include <IMP/kernel/kernel_config.h>
#include "../Particle.h"
#include "../ParticleTuple.h"
#include "../Restraint.h"
#include "../SingletonScore.h"
#include "../macros.h"
#include <IMP/base/internal/swig_helpers.h>

using namespace IMP;

#ifndef SWIG

  template<>
  struct Convert<Particle >:
    public ConvertObjectBase<Particle> {
    static const int converter=2;
    template <class SwigData>
    static Particle* get_cpp_object(PyObject *o, SwigData st,
                             SwigData particle_st, SwigData decorator_st) {
      void *vp;
      int res=SWIG_ConvertPtr(o, &vp,particle_st, 0 );
      Particle *p=nullptr;
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
            p=nullptr;
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
                 typename enable_if< and_< boost::is_base_of<Decorator, T>,
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

#endif


#endif  /* IMPKERNEL_INTERNAL_SWIG_HELPERS_H */
