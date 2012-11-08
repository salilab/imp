/**
 *  \file internal/core_particle_triplet_helpers.h
 *  \brief A container for Triplets.
 *
 *  WARNING This file was generated from ListLikeNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_LIST_LIKE_TRIPLET_CONTAINER_H
#define IMPKERNEL_INTERNAL_LIST_LIKE_TRIPLET_CONTAINER_H

#include "../kernel_config.h"
#include "../TripletContainer.h"
#include "../TripletModifier.h"
#include "../TripletScore.h"
#include "../scoped.h"
#include "TupleRestraint.h"
#include "container_helpers.h"
#include "triplet_helpers.h"
#include <algorithm>


IMP_BEGIN_INTERNAL_NAMESPACE

class IMPEXPORT ListLikeTripletContainer: public TripletContainer {
private:
  ParticleIndexTriplets data_;
protected:
  void swap(ParticleIndexTriplets &cur) {
    Container::set_is_changed(true);
    IMP::base::swap(data_, cur);
  }
  /*  template <class F>
  void remove_from_list_if(F f) {
    data_.erase(std::remove_if(data_.begin(), data_.end(), f), data_.end());
    Container::set_is_changed(true);
    }*/
  ListLikeTripletContainer(Model *m, std::string name):
    TripletContainer(m,name) {
  }
 public:
  template <class F>
    F for_each(F f) {
    return std::for_each(data_.begin(), data_.end(), f);
  }
  void apply(const TripletModifier *sm) const {
    sm->apply_indexes(get_model(), data_);
  }
  void apply(const TripletDerivativeModifier *sm,
             DerivativeAccumulator &da) const {
    sm->apply_indexes(get_model(), data_, da);
  }
  double evaluate(const TripletScore *s,
                  DerivativeAccumulator *da) const {
    return s->evaluate_indexes(get_model(), data_, da);
  }
  double evaluate_if_good(const TripletScore *s,
                          DerivativeAccumulator *da,
                          double max) const {
    return s->evaluate_if_good_indexes(get_model(), data_, da, max);
  }
  IMP_OBJECT(ListLikeTripletContainer);

  ParticleIndexTriplets get_indexes() const {
    return data_;
  }
  bool get_provides_access() const {return true;}
  const ParticleIndexTriplets& get_access() const {
    return data_;
  }

  typedef ParticleIndexTriplets::const_iterator const_iterator;
  const_iterator begin() const {
    return data_.begin();
  }
  const_iterator end() const {
    return data_.end();
  }
};


IMP_END_INTERNAL_NAMESPACE

#define IMP_LISTLIKE_TRIPLET_CONTAINER(Name)                         \
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
  ParticleIndexTriplets get_range_indexes() const;                     \
  IMP_OBJECT(Name)


#define IMP_LISTLIKE_TRIPLET_CONTAINER_2(Name)                         \
  public:                                                               \
  ParticleIndexes get_all_possible_indexes() const;                     \
  ModelObjectsTemp do_get_inputs() const;                               \
  void do_before_evaluate();                                            \
  ParticleIndexTriplets get_range_indexes() const;                     \
  IMP_OBJECT(Name)


#endif  /* IMPKERNEL_INTERNAL_LIST_LIKE_TRIPLET_CONTAINER_H */
