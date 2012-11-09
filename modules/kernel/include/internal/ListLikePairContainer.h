/**
 *  \file internal/core_particle_pair_helpers.h
 *  \brief A container for Pairs.
 *
 *  WARNING This file was generated from ListLikeNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_LIST_LIKE_PAIR_CONTAINER_H
#define IMPKERNEL_INTERNAL_LIST_LIKE_PAIR_CONTAINER_H

#include "../kernel_config.h"
#include "../PairContainer.h"
#include "../PairModifier.h"
#include "../PairScore.h"
#include "../scoped.h"
#include "TupleRestraint.h"
#include "container_helpers.h"
#include "pair_helpers.h"
#include <algorithm>


IMP_BEGIN_INTERNAL_NAMESPACE

class IMPEXPORT ListLikePairContainer: public PairContainer {
private:
  ParticleIndexPairs data_;
protected:
  void swap(ParticleIndexPairs &cur) {
    Container::set_is_changed(true);
    IMP::base::swap(data_, cur);
  }
  /*  template <class F>
  void remove_from_list_if(F f) {
    data_.erase(std::remove_if(data_.begin(), data_.end(), f), data_.end());
    Container::set_is_changed(true);
    }*/
  ListLikePairContainer(Model *m, std::string name):
    PairContainer(m,name) {
  }
 public:
  template <class F>
  void apply_generic(const F *f) const {
    f->apply_indexes(get_model(), data_);
  }
  void apply(const PairModifier *sm) const {
    apply_generic(sm);
  }
  IMP_OBJECT(ListLikePairContainer);

  ParticleIndexPairs get_indexes() const {
    return data_;
  }
  bool get_provides_access() const {return true;}
  const ParticleIndexPairs& get_access() const {
    return data_;
  }

  typedef ParticleIndexPairs::const_iterator const_iterator;
  const_iterator begin() const {
    return data_.begin();
  }
  const_iterator end() const {
    return data_.end();
  }
};


IMP_END_INTERNAL_NAMESPACE

#define IMP_LISTLIKE_PAIR_CONTAINER(Name)                         \
  public:                                                               \
  ParticleIndexes get_all_possible_indexes() const;                     \
  ParticlesTemp get_input_particles() const;                            \
  ContainersTemp get_input_containers() const;                          \
  ModelObjectsTemp do_get_inputs() const {                              \
    ModelObjects ret;                                                   \
    ret+=get_input_containers();                                        \
    ret+=get_input_particles();                                         \
    return ret;                                                         \
  }                                                                     \
  void do_before_evaluate();                                            \
  ParticleIndexPairs get_range_indexes() const;                     \
  IMP_OBJECT(Name)


#define IMP_LISTLIKE_PAIR_CONTAINER_2(Name)                         \
  public:                                                               \
  ParticleIndexes get_all_possible_indexes() const;                     \
  ModelObjectsTemp do_get_inputs() const;                               \
  void do_before_evaluate();                                            \
  ParticleIndexPairs get_range_indexes() const;                     \
  IMP_OBJECT(Name)


#endif  /* IMPKERNEL_INTERNAL_LIST_LIKE_PAIR_CONTAINER_H */
