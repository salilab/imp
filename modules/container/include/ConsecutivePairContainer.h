/**
 *  \file IMP/container/ConsecutivePairContainer.h
 *  \brief Return all pairs from a SingletonContainer
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H
#define IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H

#include <IMP/container/container_config.h>
#include <IMP/generic.h>
#include <IMP/PairContainer.h>
#include <IMP/PairPredicate.h>
#include <IMP/SingletonContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/base/map.h>
#include <IMP/pair_macros.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! A container which contains all consecutive pairs from an input  list
//  of particles
/** If it is assumed that each particle is in at most one such container,
    then ExclusiveConsecutivePairContainer should be used instead,
    since it is faster when doing certain computations.

    Also see ConsecutivePairFilter.
*/
class IMPCONTAINEREXPORT ConsecutivePairContainer : public PairContainer {
  friend class ConsecutivePairFilter;
  const kernel::ParticleIndexes ps_;
  IntKey key_;
  /**
     add the key of this container as an attribute to all particles
     if there might be ovrlaps - create a different keys for each instance
  */
  void init();

  bool get_contains(const kernel::ParticleIndexPair &p) const {
    if (!get_model()->get_has_attribute(key_, p[0])) return false;
    int ia = get_model()->get_attribute(key_, p[0]);
    if (!get_model()->get_has_attribute(key_, p[1])) return false;
    int ib = get_model()->get_attribute(key_, p[1]);
    return std::abs(ia - ib) == 1;
  }

 public:
  //! apply to each item in container
  template <class F>
  void apply_generic(F *f) const {
    for (unsigned int i = 1; i < ps_.size(); ++i) {
      f->apply_index(get_model(),
                     kernel::ParticleIndexPair(ps_[i - 1], ps_[i]));
    }
  }
  //! Get the individual particles from the passed SingletonContainer
  ConsecutivePairContainer(const kernel::ParticlesTemp &ps,
                           std::string name = "ConsecutivePairContainer%1%");
  virtual kernel::ParticleIndexPairs get_indexes() const IMP_OVERRIDE;
  virtual kernel::ParticleIndexPairs get_range_indexes() const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual kernel::ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;
  virtual void do_before_evaluate() IMP_OVERRIDE;
  IMP_PAIR_CONTAINER_METHODS(ConsecutivePairContainer);
  IMP_OBJECT_METHODS(ConsecutivePairContainer);
};

IMP_OBJECTS(ConsecutivePairContainer, ConsecutivePairContainers);

/** Check for whether the pair is a member of a specific
    ConsecutivePairContainer. */
class IMPCONTAINEREXPORT ConsecutivePairFilter : public PairPredicate {
  base::PointerMember<ConsecutivePairContainer> cpc_;

 public:
  ConsecutivePairFilter(ConsecutivePairContainer *cpc);

  virtual int get_value_index(kernel::Model *,
                              const kernel::ParticleIndexPair &pip) const
      IMP_OVERRIDE {
    return cpc_->get_contains(pip);
  }
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pi) const IMP_OVERRIDE {
    kernel::ModelObjectsTemp ret;
    ret += IMP::get_particles(m, pi);
    return ret;
  }
  IMP_PAIR_PREDICATE_METHODS(ConsecutivePairFilter);
  IMP_OBJECT_METHODS(ConsecutivePairFilter);
};

/** This is an ConsecutivePairContainer where each particle can only be on
    one ExclusiveConsecutivePairContainer. The exclusivity makes the code
    more efficient and allows one to use the ExclusiveConsecutivePairFilter,
    which is way more efficient than using an InContainerPairFilter
    with a ConsecutivePairContainer.*/
class IMPCONTAINEREXPORT ExclusiveConsecutivePairContainer
    : public PairContainer {
  friend class ExclusiveConsecutivePairFilter;
  const kernel::ParticleIndexes ps_;
  static IntKey get_exclusive_key() {
    static IntKey k("exclusive consecutive numbering");
    return k;
  }
  static ObjectKey get_exclusive_object_key() {
    static ObjectKey k("exclusive consecutive container");
    return k;
  }
  static bool get_contains(kernel::Model *m,
                           const kernel::ParticleIndexPair &pp) {
    ObjectKey ok =
        ExclusiveConsecutivePairContainer::get_exclusive_object_key();
    if (!m->get_has_attribute(ok, pp[0]) || !m->get_has_attribute(ok, pp[1]))
      return false;
    if (m->get_attribute(ok, pp[0]) != m->get_attribute(ok, pp[1])) {
      return false;
    }
    IntKey k = ExclusiveConsecutivePairContainer::get_exclusive_key();
    int ia = m->get_attribute(k, pp[0]);
    int ib = m->get_attribute(k, pp[1]);
    return std::abs(ia - ib) == 1;
  }
  void init();

 public:
  //! apply to each item in container
  template <class F>
  void apply_generic(F *f) const {
    for (unsigned int i = 1; i < ps_.size(); ++i) {
      f->apply_index(get_model(),
                     kernel::ParticleIndexPair(ps_[i - 1], ps_[i]));
    }
  }

  //! Get the individual particles from the passed SingletonContainer
  ExclusiveConsecutivePairContainer(const kernel::ParticlesTemp &ps,
                                    std::string name =
                                        "ExclusiveConsecutivePairContainer%1%");
  virtual kernel::ParticleIndexPairs get_indexes() const IMP_OVERRIDE;
  virtual kernel::ParticleIndexPairs get_range_indexes() const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual kernel::ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;
  virtual void do_before_evaluate() IMP_OVERRIDE;
  IMP_PAIR_CONTAINER_METHODS(ExclusiveConsecutivePairContainer);
  IMP_OBJECT_METHODS(ExclusiveConsecutivePairContainer);
};

/** Check for whether the pair is a member of any
    ExclusiveConsecutivePairContainer. */
class IMPCONTAINEREXPORT ExclusiveConsecutivePairFilter : public PairPredicate {
 public:
  ExclusiveConsecutivePairFilter()
      : PairPredicate("ExclusiveConsecutivePairFilter %1%") {}

  virtual int get_value_index(kernel::Model *m,
                              const kernel::ParticleIndexPair &pip) const
      IMP_OVERRIDE {
    return ExclusiveConsecutivePairContainer::get_contains(m, pip);
  }
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pi) const IMP_OVERRIDE {
    kernel::ModelObjectsTemp ret;
    ret += IMP::get_particles(m, pi);
    return ret;
  }
  IMP_PAIR_PREDICATE_METHODS(ExclusiveConsecutivePairFilter);
  IMP_OBJECT_METHODS(ExclusiveConsecutivePairFilter);
};

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H */
