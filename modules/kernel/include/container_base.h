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
    - user-provided lists, e.g. IMP::container::ListSingletonContainer
    - operations on other containers e.g. IMP::container::PairContainerSet
    - computations based on particle attributes
      e.g. IMP::container::ClosePairContainer

    Most basically, containers allow you to get their contents
    (eg SingletonContainer::get_indexes()) or do an operation on their contents
    IMP_CONTAINER_FOREACH().

    \note Containers store \em sets and so are fundamentally unordered.
 */
class IMPKERNELEXPORT Container : public ScoreState {
  int version_;
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  bool readable_;
  bool writeable_;
#endif
 protected:
  //! Call this with true when the contents of the container change
  /** See get_contents_version() to monitor the container for changes.

      Static containers should call this in their constructor. */
  void set_is_changed(bool tf);
  Container(kernel::Model *m, std::string name = "Container %1%");

 public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  // lazy, should go in an internal header
  static bool update_version(Container *c, int &version) {
    int old = version;
    version = c->get_contents_version();
    return old != version;
  }
#endif
  //! Get contained particles
  /** Get a list of all particles contained in this one,
      given that the input containers are up to date.
  */
  virtual ParticleIndexes get_all_possible_indexes() const = 0;

  /** Return true if the container changed since the last evaluate.

      \deprecated_at{2.1} Use get_contents_version() instead as that
      is safer.*/
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  bool get_is_changed() const;

  /** Return a counter that can be used to detect when the contents
      of the container changed. Store the value and then compare
      against the version next time to detect if it is different. It
      is initially 0.*/
  int get_contents_version() const { return version_; }

  //! containers don't have outputs
  ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }

  //! Reset changed status
  virtual void do_after_evaluate(DerivativeAccumulator *accpt) IMP_OVERRIDE;

  /** True if the container's contents are not independent from one
      another, and so it cannot be decomposed into a sum of terms.
      Examples include connectivity.*/
  virtual bool get_is_decomposable() const { return true; }

#if !defined(IMP_DOXYGEN)
  // methods to implement checking for inputs and outputs
  void validate_readable() const;
  void validate_writable() const;
  void set_is_readable(bool tf);
  void set_is_writable(bool tf);
#endif

  /** \deprecated_at{2.1} use get_all_possible_indexes() instead
   */
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
  ParticlesTemp get_all_possible_particles() const {
    IMPKERNEL_DEPRECATED_METHOD_DEF(
        2.1,
        "Use IMP::Container::get_all_possible_indexes()"
        "instead");
    return IMP::kernel::get_particles(get_model(), get_all_possible_indexes());
  }

  IMP_REF_COUNTED_DESTRUCTOR(Container);
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CONTAINER_BASE_H */
