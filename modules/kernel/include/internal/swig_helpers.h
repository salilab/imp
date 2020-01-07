/**
 *  \file internal/swig_helpers.h
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_SWIG_HELPERS_H
#define IMPKERNEL_INTERNAL_SWIG_HELPERS_H

// Python.h must be included first
#include <IMP/kernel_config.h>
#include "../Particle.h"
#include "../Restraint.h"
#include "../SingletonScore.h"
#include "../macros.h"
#include <IMP/internal/swig_helpers_base.h>

using namespace IMP;

#ifndef SWIG

template <>
struct Convert<Particle> : public ConvertObjectBase<Particle> {
  static const int converter = 2;
  template <class SwigData>
  static Particle *get_cpp_object(PyObject *o, const char *symname, int argnum,
                                  const char *argtype, SwigData,
                                  SwigData particle_st, SwigData decorator_st) {
    void *vp;
    int res = SWIG_ConvertPtr(o, &vp, particle_st, 0);
    Particle *p = nullptr;
    if (!SWIG_IsOK(res)) {
      int res = SWIG_ConvertPtr(o, &vp, decorator_st, 0);
      if (!SWIG_IsOK(res)) {
        IMP_THROW(get_convert_error("Wrong type", symname, argnum, argtype),
                  TypeException);
      } else {
        Decorator *d = reinterpret_cast<Decorator *>(vp);
        if (*d) {
          p = d->get_particle();
        } else {
          p = nullptr;
        }
      }
    } else {
      p = reinterpret_cast<Particle *>(vp);
    }
    if (p) {
      IMP_CHECK_OBJECT(static_cast<Object *>(p));
    }
    return p;
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject *o, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    try {
      get_cpp_object(o, "", 0, "", st, particle_st, decorator_st);
    }
    catch (...) {
      return 0;
    }
    return 1;
  }
};

template <>
struct Convert<ParticleIndex> : public ConvertValueBase<ParticleIndex> {
  typedef Convert<Particle> Helper;
  static const int converter = 40;
  template <class SwigData>
  static ParticleIndex get_cpp_object(PyObject *o, const char *symname,
                                      int argnum, const char *argtype,
                                      SwigData index_st,
                                      SwigData particle_st,
                                      SwigData decorator_st) {
    void *vp;
    int res = SWIG_ConvertPtr(o, &vp, index_st, 0);
    if (SWIG_IsOK(res)) {
      ParticleIndex *temp = reinterpret_cast<ParticleIndex *>(vp);
      ParticleIndex ret = *temp;
      if (SWIG_IsNewObj(res)) delete temp;
      return ret;
    } else {
      Particle *p = Helper::get_cpp_object(o, symname, argnum, argtype,
                                           index_st, particle_st,
                                           decorator_st);
      return p->get_index();
    }
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject *o, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    try {
      get_cpp_object(o, "", 0, "", st, particle_st, decorator_st);
    }
    catch (...) {
      return 0;
    }
    return 1;
  }
};

template <class T>
struct Convert<
    T, typename enable_if<and_<boost::is_base_of<Decorator, T>,
                               not_<typename T::DecoratorHasTraits> > >::
           type> : public ConvertValueBase<T> {
  static const int converter = 3;
  template <class SwigData>
  static T get_cpp_object(PyObject *o, const char *symname, int argnum,
                          const char *argtype, SwigData, SwigData particle_st,
                          SwigData decorator_st) {
    Particle *p = Convert<Particle>::get_cpp_object(o, symname, argnum,
                                                    argtype, particle_st,
                                                    particle_st, decorator_st);
    if (!T::get_is_setup(p)) {
      std::ostringstream msg;
      msg << "Particle " << p->get_name()
          << " is not of correct decorator type";
      IMP_THROW(get_convert_error(msg.str().c_str(), symname, argnum, argtype),
                ValueException);
    }
    return T(p);
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject *o, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    try {
      get_cpp_object(o, "", 0, "", st, particle_st, decorator_st);
    }
    catch (...) {
      return 0;
    }
    return 1;
  }
};

template <class T>
struct Convert<
    T, typename enable_if<
           typename T::DecoratorHasTraits>::type> : public ConvertValueBase<T> {
  static const int converter = 4;
  template <class SwigData>
  static T get_cpp_object(PyObject *o, const char *symname, int argnum,
                          const char *argtype, SwigData st,
                          SwigData particle_st, SwigData decorator_st) {
    try {
      return ConvertValueBase<T>::get_cpp_object(o, symname, argnum, argtype,
                                                 st, particle_st, decorator_st);
    }
    catch (ValueException) {
      Particle *p = Convert<Particle>::get_cpp_object(
          o, symname, argnum, argtype, particle_st, particle_st, decorator_st);
      if (!T::get_is_setup(p)) {
        std::ostringstream msg;
        msg << "Particle " << p->get_name()
            << " is not of correct decorator type";
        IMP_THROW(get_convert_error(msg.str().c_str(), symname, argnum,
                                    argtype),
                  ValueException);
      }
      return T(p);
    }
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject *o, SwigData st, SwigData particle_st,
                                SwigData decorator_st) {
    try {
      get_cpp_object(o, "", 0, "", st, particle_st, decorator_st);
    }
    catch (...) {
      return 0;
    }
    return 1;
  }
};

#endif

#endif /* IMPKERNEL_INTERNAL_SWIG_HELPERS_H */
