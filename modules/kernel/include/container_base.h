/**
 *  \file IMP/kernel/container_base.h
 *  \brief Abstract base class for containers of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONTAINER_BASE_H
#define IMPKERNEL_CONTAINER_BASE_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "ScoreState.h"
#include "particle_index.h"
#include <IMP/base/utility_macros.h>
#include <IMP/base/ref_counted_macros.h>
#include <IMP/base/Object.h>
#include <IMP/base/WeakPointer.h>

IMPKERNEL_BEGIN_NAMESPACE
class Particle;
class Model;

//! Abstract class for containers of particles
/** Containers store sets of tuples of particles. The degree of the tuple
    (i.e. whether each tuple contains one, two, three or four
    particles) is constant for each container. That is, a
    SingletonContainer is a set of single particles, a PairContainer
    is a set of pairs of particles etc.

    These sets can come from a variety of sources, such as
    - user-provided lists, e.g. IMP::core::ListSingletonContainer
    - operations on other containers e.g. IMP::core::PairContainerSet
    - computations based on particle attributes
      e.g. IMP::Core::ClosePairContainer

    Containers provide a variety of methods to
    - get the number of tuples
    - get the ith tuple
    - iterate through the tuples
    - determine if a tuple is contained in the set

    \note If nothing uses the added and removed containers they may not
    be updated.

    \note Containers store \em sets and so are fundamentally unordered.
 */
class IMPKERNELEXPORT Container : public ScoreState
{
  bool changed_;
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  bool readable_;
  bool writeable_;
#endif
protected:
  //! This will be reset at the end of evaluate
  void set_is_changed(bool tf);
  Container (Model *m, std::string name="Container %1%");
 public:
  //! Get contained particles
  /** Get a list of all particles contained in this one,
      given that the input containers are up to date.
  */
  virtual ParticleIndexes get_all_possible_indexes() const=0;

  /** \deprecated use get_all_possible_indexes() instead
   */
  IMP_DEPRECATED_WARN
    ParticlesTemp get_all_possible_particles() const {
    IMP_DEPRECATED_FUNCTION(Use IMP::Container::get_all_possible_indexes()
                            instead);
    return IMP::kernel::get_particles(get_model(), get_all_possible_indexes());
  }

  /** Return true if the container changed since the last evaluate.*/
  bool get_is_changed() const;

  //! containers don't have outputs
  ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }

  //! Reset changed status
  virtual void do_after_evaluate(DerivativeAccumulator *accpt) IMP_OVERRIDE;

  /** True if the container's contents are not independent from one
      another, and so it cannot be decomposed into a sum of terms.
      Examples include connectivity.*/
  virtual bool get_is_decomposable() const {return true;}

#if !defined(IMP_DOXYGEN)
  // methods to implement checking for inputs and outputs
  void validate_readable() const;
  void validate_writable() const;
  void set_is_readable(bool tf);
  void set_is_writable(bool tf);
#endif
  IMP_REF_COUNTED_DESTRUCTOR(Container);
};

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_CONTAINER_BASE_H */
