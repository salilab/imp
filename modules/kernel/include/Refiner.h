/**
 *  \file IMP/kernel/Refiner.h
 *  \brief Refine a particle into a list of particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_REFINER_H
#define IMPKERNEL_REFINER_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "Particle.h"
#include "internal/IndexingIterator.h"
#include <IMP/base/deprecation_macros.h>
#include <IMP/base/check_macros.h>
#include "model_object_helpers.h"

IMPKERNEL_BEGIN_NAMESPACE

class Particle;
class DerivativeAccumulator;

//! Abstract class to implement hierarchical methods.
/** The job of this class is to take a single particle and, if
    appropriate, return a list of particles. These lists can
    reflect existing relationships, such as the
    IMP::core::LeavesRefiner or arbitrary relationships set up
    for a particular purpose, such as IMP::core::TableRefiner.

    @note it is assumed that refined particles belong to the same model
          as the coarse particle
*/
class IMPKERNELEXPORT Refiner : public ParticleInputs, public base::Object {
  struct Accessor;
  bool is_by_ref_;

 public:
  /** Constructs the refiner

      @param name object name for refiner
      @param is_by_ref if true, this refiner is expected to support
                       the get_refined_indexes_by_ref method,
                       for refiners that support faster cached
                       list of particles, etc.
  */
  Refiner(std::string name = "Refiner %1%", bool is_by_ref = false);
  //! Return true if this refiner can refine that particle
  /** This should not throw, so be careful what fields are touched.
   */
  virtual bool get_can_refine(Particle *) const { return false; }

  //! Refine the passed particle into a set of particles.
  /** As a precondition can_refine_particle(a) should be true.

      @param a coarse particle to be refined
   */
  virtual const ParticlesTemp get_refined(Particle *a) const = 0;

  //! Return the indexes of the particles returned by get_refined()
  /** Return the indexes of the particles returned by get_refined()
      for particle pi in model m.

      @param m,pi model and particle index of coarse particle to be refined

      @note assumes that the refined particles are also in model m
   */
  virtual ParticleIndexes get_refined_indexes
    (Model *m, ParticleIndex pi) const;

  //! Return the indexes of the particles returned by get_refined()
  /** Return the indexes of the particles returned by get_refined()
      for particle pi in model m by reference (possible faster).

      @param m,pi model and particle index of coarse particle to be refined

      @note assumes that get_is_by_ref() is true.
      @note assumes that the refined particles are also in model m
   */
  virtual ParticleIndexes const& get_refined_indexes_by_ref
    (Model *m, ParticleIndex pi) const
  {
    IMP_ALWAYS_CHECK(false,
                    "This refiner does not support"
                     " get_refined_indexes_by_ref()",
                     base::ValueException);
    IMP_UNUSED(m);
    IMP_UNUSED(pi);
  }

  //! returns true if this refiner supports
  //! get_refined_indexes_by_ref() (e.g. FixedRefiner)
  bool get_is_by_ref_supported() { return is_by_ref_; };

  //! Get the ith refined particle.
  /** As a precondition can_refine_particle(a) should be true.
   */
  virtual Particle *get_refined(Particle *a, unsigned int i) const {
    return get_refined(a)[i];
  }

  /** As a precondition can_refine_particle(a) should be true.
   */
  virtual unsigned int get_number_of_refined(Particle *a) const {
    return get_refined(a).size();
  }

#ifndef SWIG
  /** @name Iterating through the set of refined particles

      Using iterators can be more efficient than using the bulk
      get_refined(), however it is not necessarily so.
      @{
  */
  typedef internal::IndexingIterator<Accessor> RefinedIterator;
  RefinedIterator refined_begin(Particle *a) const;
  RefinedIterator refined_end(Particle *a) const;
/** @} */
#endif
};
//! a collection of Refiner objects
IMP_OBJECTS(Refiner, Refiners);

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
struct Refiner::Accessor {
  // can't reference count since swig memory management is broken
  Particle *p_;
  const Refiner *r_;
  Accessor(Particle *p, const Refiner *r) : p_(p), r_(r) {}
  Accessor() {}
  typedef Particle *result_type;
  Particle *operator()(unsigned int i) const { return r_->get_refined(p_, i); }
  bool operator==(const Accessor &o) const { return p_ == o.p_ && r_ == o.r_; }
};
#endif

inline Refiner::RefinedIterator Refiner::refined_begin(Particle *a) const {
  return RefinedIterator(Accessor(a, this), 0);
}
inline Refiner::RefinedIterator Refiner::refined_end(Particle *a) const {
  return RefinedIterator(Accessor(a, this), get_number_of_refined(a));
}

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_REFINER_H */
