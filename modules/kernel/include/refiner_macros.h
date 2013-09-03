/**
 *  \file IMP/kernel/refiner_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_REFINER_MACROS_H
#define IMPKERNEL_REFINER_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include "Refiner.h"
#include "container_base.h"
#include <IMP/base/object_macros.h>

/** \deprecated_at{2.1} Declare methods yourself.
*/
#define IMP_REFINER(Name)                                        \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare methods yourself");   \
  virtual bool get_can_refine(Particle *) const;                 \
  virtual const ParticlesTemp get_refined(Particle *) const;     \
  virtual Particle *get_refined(Particle *, unsigned int) const; \
  virtual unsigned int get_number_of_refined(Particle *) const;  \
  IMP_BACKWARDS_MACRO_INPUTS;                                    \
  IMP_OBJECT_NO_WARNING(Name)

/** \deprecated_at{2.1} Declare methods yourself.
*/
#define IMP_SIMPLE_REFINER(Name)                                 \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare methods yourself");   \
  virtual bool get_can_refine(Particle *) const;                 \
  virtual Particle *get_refined(Particle *, unsigned int) const; \
  virtual const ParticlesTemp get_refined(Particle *a) const {   \
    ParticlesTemp ret(get_number_of_refined(a));                 \
    for (unsigned int i = 0; i < ret.size(); ++i) {              \
      ret[i] = get_refined(a, i);                                \
    }                                                            \
    return ret;                                                  \
  }                                                              \
  virtual unsigned int get_number_of_refined(Particle *) const;  \
  IMP_BACKWARDS_MACRO_INPUTS;                                    \
  IMP_OBJECT_NO_WARNING(Name)

/** \deprecated_at{2.1} Just define the methods yourself.
*/
#define IMP_SINGLETON_MODIFIER_FROM_REFINED(Name, refiner)                     \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Define methods yourself");                  \
  ModelObjectsTemp Name::do_get_inputs(kernel::Model *m,                               \
                                       const ParticleIndexes &pis) const {     \
    ModelObjectsTemp ret = refiner->get_inputs(m, pis);                        \
    ret += IMP::kernel::get_particles(m, pis);                                 \
    for (unsigned int i = 0; i < pis.size(); ++i) {                            \
      ret +=                                                                   \
          IMP::kernel::get_particles(refiner->get_refined_indexes(m, pis[i])); \
    }                                                                          \
    return ret;                                                                \
  }                                                                            \
  ModelObjectsTemp Name::do_get_outputs(kernel::Model *m,                              \
                                        const ParticleIndexes &pis) const {    \
    ModelObjectsTemp ret = IMP::kernel::get_particles(m, pis);                 \
    return ret;                                                                \
  }                                                                            \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

/** \deprecated_at{2.1} Just define the methods yourself.
*/
#define IMP_SINGLETON_MODIFIER_TO_REFINED(Name, refiner)                       \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Define methods yourself");                  \
  ModelObjectsTemp Name::do_get_inputs(kernel::Model *m,                               \
                                       const ParticleIndexes &pis) const {     \
    ModelObjectsTemp ret = refiner->get_inputs(m, pis);                        \
    for (unsigned int i = 0; i < pis.size(); ++i) {                            \
      ret +=                                                                   \
          IMP::kernel::get_particles(refiner->get_refined_indexes(m, pis[i])); \
    }                                                                          \
    ret += IMP::kernel::get_particles(m, pis);                                 \
    return ret;                                                                \
  }                                                                            \
  ModelObjectsTemp Name::do_get_outputs(kernel::Model *m,                              \
                                        const ParticleIndexes &pis) const {    \
    ModelObjectsTemp ret;                                                      \
    for (unsigned int i = 0; i < pis.size(); ++i) {                            \
      ret +=                                                                   \
          IMP::kernel::get_particles(refiner->get_refined_indexes(m, pis[i])); \
    }                                                                          \
    return ret;                                                                \
  }                                                                            \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#endif /* IMPKERNEL_REFINER_MACROS_H */
