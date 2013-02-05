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



//! Define the basics needed for a particle refiner
/** In addition to the methods done by all the macros, it declares
    - IMP::Refiner::get_can_refine()
    - IMP::Refiner::get_number_of_refined()
    - IMP::Refiner::get_refined()
    - IMP::Refiner::get_input_particles()
    \see IMP_SIMPLE_REFINER
*/
#define IMP_REFINER(Name)                                               \
  virtual bool get_can_refine(Particle*) const;                         \
  virtual const ParticlesTemp get_refined(Particle *) const;            \
  virtual Particle* get_refined(Particle *, unsigned int) const;        \
  virtual unsigned int get_number_of_refined(Particle *) const;         \
  IMP_BACKWARDS_MACRO_INPUTS;                                           \
  IMP_OBJECT(Name)


//! Define the basics needed for a particle refiner
/** In contrast to IMP_REFINER, if this macro is used, the
    Refiner::get_refined(Particle*) method is implemented using the
    other Refiner::get_refined() method and so does not have to be
    provided.

    \see IMP_REFINER
*/
#define IMP_SIMPLE_REFINER(Name)                                        \
  virtual bool get_can_refine(Particle*) const;                         \
  virtual Particle* get_refined(Particle *, unsigned int) const;        \
  virtual const ParticlesTemp get_refined(Particle *a) const {          \
    ParticlesTemp ret(get_number_of_refined(a));                        \
    for (unsigned int i=0; i< ret.size(); ++i) {                        \
      ret[i]= get_refined(a,i);                                         \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  virtual unsigned int get_number_of_refined(Particle *) const;         \
  IMP_BACKWARDS_MACRO_INPUTS;                                           \
  IMP_OBJECT(Name)









//! Add interaction methods to a SingletonModifer
/** This macro is designed to be used in conjunction with
    IMP_SINGLETON_MODIFIER or IMP_SINGLETON_MODIFIER_DA. It adds
    definitions for the methods:
    - IMP::SingletonModifier::get_input_particles()
    - IMP::SingletonModifier::get_output_particles()
    for a modifier which updates the passed particle based on the results
    of refinement.
*/
#define IMP_SINGLETON_MODIFIER_FROM_REFINED(Name, refiner)              \
  ModelObjectsTemp Name::do_get_inputs(Model *m,                        \
                                       const ParticleIndexes &pis) const { \
    ModelObjectsTemp ret;                                               \
    ret+= refiner->get_inputs(m, pis);                                  \
    ret+= IMP::kernel::get_particles(m, pis);                           \
    for (unsigned int i=0; i< pis.size(); ++i) {                        \
      ret+=refiner->get_refined(m->get_particle(pis[i]));               \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  ModelObjectsTemp Name::do_get_outputs(Model *m,                       \
                                        const ParticleIndexes &pis) const { \
    ModelObjectsTemp ret;                                               \
    ret+=IMP::kernel::get_particles(m, pis);                            \
    return ret;                                                         \
  }                                                                     \
  IMP_NO_DOXYGEN(void Name::do_show(std::ostream &out) const {          \
    out <<"refiner " << *refiner << std::endl;                          \
    })                                                                  \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

//! Add interaction methods to a SingletonModifer
/** This macro is designed to be used in conjunction with
    IMP_SINGLETON_MODIFIER or IMP_SINGLETON_MODIFIER_DA. It adds
    definitions for the methods:
    - IMP::SingletonModifier::get_input_particles()
    - IMP::SingletonModifier::get_output_particles()
    - IMP::base::Object::do_show()
    for a modifier which updates the refined particles based on the one
    they are refined from.

    This macro should appear in a .cpp file.
*/
#define IMP_SINGLETON_MODIFIER_TO_REFINED(Name, refiner)                \
  ModelObjectsTemp Name::do_get_inputs(Model *m,                        \
                                       const ParticleIndexes &pis) const { \
    ModelObjectsTemp ret;                                               \
    ret+= refiner->get_inputs(m, pis);                                  \
    for (unsigned int i=0; i< pis.size(); ++i) {                        \
      ret+=refiner->get_refined(m->get_particle(pis[i]));               \
    }                                                                   \
    ret+= IMP::kernel::get_particles(m, pis);                           \
    return ret;                                                         \
  }                                                                     \
  ModelObjectsTemp Name::do_get_outputs(Model *m,                       \
                                        const ParticleIndexes &pis) const { \
    ModelObjectsTemp ret;                                               \
    for (unsigned int i=0; i< pis.size(); ++i) {                        \
      ret+=refiner->get_refined(m->get_particle(pis[i]));               \
    }                                                                   \
    return ret;                                                         \
   }                                                                     \
  IMP_NO_DOXYGEN(void Name::do_show(std::ostream &out) const {          \
    out << "refiner " << *refiner << std::endl;                         \
    })                                                                  \
  IMP_REQUIRE_SEMICOLON_NAMESPACE



#endif  /* IMPKERNEL_REFINER_MACROS_H */
