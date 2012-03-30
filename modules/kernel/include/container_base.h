/**
 *  \file IMP/container_base.h
 *  \brief Abstract base class for containers of particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONTAINER_BASE_H
#define IMPKERNEL_CONTAINER_BASE_H

#include "kernel_config.h"
#include "base_types.h"
#include "Constraint.h"
#include <IMP/base/ref_counted_macros.h>
#include <IMP/base/Object.h>
#include <IMP/base/WeakPointer.h>

IMP_BEGIN_NAMESPACE
class Particle;
template <unsigned int D>
class ParticleTuple;
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
class IMPEXPORT Container : public Constraint
{
  bool changed_;
 protected:
  //! This will be reset at the end of evaluate
  void set_is_changed(bool tf);
  Container(Model *m, std::string name="Container %1%");
 public:
  //! Get contained particles
  /** Get a list of all particles contained in this one,
      given that the input containers are up to date.
  */
  virtual ParticlesTemp get_all_possible_particles() const=0;

  /** Return true if the container changed since the last evaluate.*/
  virtual bool get_is_changed() const {return changed_;}
  //! Return get_all_possible_particles()
  virtual ParticlesTemp get_input_particles() const;
  //! Containers don't have output
  virtual ParticlesTemp get_output_particles() const;
  //! Containers don't have output
  virtual ContainersTemp get_output_containers() const;
  //! Reset changed status
  virtual void do_after_evaluate(DerivativeAccumulator *accpt);

  IMP_REF_COUNTED_DESTRUCTOR(Container);
};

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_CONTAINER_BASE_H */
