/**
 *  \file IMP/container_base.h
 *  \brief Abstract base class for containers of particles.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CONTAINER_BASE_H
#define IMPKERNEL_CONTAINER_BASE_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include "ModelObject.h"
#include "particle_index.h"
#include <IMP/utility_macros.h>
#include <IMP/ref_counted_macros.h>
#include <IMP/Object.h>
#include <IMP/WeakPointer.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

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
    - user-provided lists, e.g. IMP::container::ListSingletonContainer
    - operations on other containers e.g. IMP::container::PairContainerSet
    - computations based on particle attributes
      e.g. IMP::container::ClosePairContainer

    Most basically, containers allow you to get their contents
    (eg SingletonContainer::get_indexes()) or do an operation on their contents
    IMP_CONTAINER_FOREACH().

    \note Containers store \em sets and so are fundamentally unordered.
 */
class IMPKERNELEXPORT Container : public ModelObject {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  bool readable_;
  bool writeable_;
#endif

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
#if IMP_HAS_CHECKS < IMP_INTERNAL
    // serialize the same data regardless of the check level, so we are portable
    bool readable_ = true, writeable_ = true;
#endif
    ar(cereal::base_class<ModelObject>(this), readable_, writeable_);
  }

 protected:
  Container(Model *m, std::string name = "Container %1%");
  Container() {}

  virtual std::size_t do_get_contents_hash() const = 0;

 public:
  //! Get contained particles
  /** Get a list of all particles contained in this one,
      given that the input containers are up to date.
  */
  virtual ParticleIndexes get_all_possible_indexes() const = 0;

  /** Return a hash that can be used to detect when the contents
      of the container changed. Store the value and then compare
      against the version next time to detect if it is different. */
  std::size_t get_contents_hash() const { return do_get_contents_hash(); }

  //! containers don't have outputs
  ModelObjectsTemp do_get_outputs() const override {
    return ModelObjectsTemp();
  }

  /** True if the container's contents are not independent from one
      another, and so it cannot be decomposed into a sum of terms.
      Examples include connectivity.*/
  virtual bool get_is_decomposable() const { return true; }

#if !defined(IMP_DOXYGEN)
  // methods to implement checking for inputs and outputs (only when checks beyond or equal internal)
  void validate_readable() const;
  void validate_writable() const;
  void set_is_readable(bool tf);
  void set_is_writable(bool tf);
#endif

  IMP_REF_COUNTED_DESTRUCTOR(Container);
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CONTAINER_BASE_H */
