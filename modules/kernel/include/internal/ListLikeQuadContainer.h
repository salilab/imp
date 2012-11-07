/**
 *  \file internal/core_particle_quad_helpers.h
 *  \brief A container for Quads.
 *
 *  WARNING This file was generated from ListLikeNAMEContainer.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_LIST_LIKE_QUAD_CONTAINER_H
#define IMPKERNEL_INTERNAL_LIST_LIKE_QUAD_CONTAINER_H

#include "../kernel_config.h"
#include "../QuadContainer.h"
#include "../QuadModifier.h"
#include "../QuadScore.h"
#include "../scoped.h"
#include "TupleRestraint.h"
#include "container_helpers.h"
#include "quad_helpers.h"
#include <algorithm>


IMP_BEGIN_INTERNAL_NAMESPACE

class IMPEXPORT ListLikeQuadContainer: public QuadContainer {
private:
  ParticleIndexQuads data_;
protected:
  void swap(ParticleIndexQuads &cur) {
    Container::set_is_changed(true);
    IMP::base::swap(data_, cur);
  }
  /*  template <class F>
  void remove_from_list_if(F f) {
    data_.erase(std::remove_if(data_.begin(), data_.end(), f), data_.end());
    Container::set_is_changed(true);
    }*/
  ListLikeQuadContainer(Model *m, std::string name):
    QuadContainer(m,name) {
  }
 public:
  template <class F>
    F for_each(F f) {
    return std::for_each(data_.begin(), data_.end(), f);
  }
  void apply(const QuadModifier *sm) const {
    sm->apply_indexes(get_model(), data_);
  }
  void apply(const QuadDerivativeModifier *sm,
             DerivativeAccumulator &da) const {
    sm->apply_indexes(get_model(), data_, da);
  }
  double evaluate(const QuadScore *s,
                  DerivativeAccumulator *da) const {
    return s->evaluate_indexes(get_model(), data_, da);
  }
  double evaluate_if_good(const QuadScore *s,
                          DerivativeAccumulator *da,
                          double max) const {
    return s->evaluate_if_good_indexes(get_model(), data_, da, max);
  }
  ParticlesTemp get_all_possible_particles() const {
    return IMP::internal::flatten(IMP::internal::get_particle(get_model(),
                                                              data_));
  }
  IMP_OBJECT(ListLikeQuadContainer);

  ParticleIndexQuads get_indexes() const {
    return data_;
  }
  bool get_provides_access() const {return true;}
  const ParticleIndexQuads& get_access() const {
    return data_;
  }

  typedef ParticleIndexQuads::const_iterator const_iterator;
  const_iterator begin() const {
    return data_.begin();
  }
  const_iterator end() const {
    return data_.end();
  }
};


IMP_END_INTERNAL_NAMESPACE

#define IMP_LISTLIKE_QUAD_CONTAINER(Name)                         \
  public:                                                               \
  ParticlesTemp get_all_possible_particles() const;                     \
  ParticlesTemp get_input_particles() const;                            \
  ContainersTemp get_input_containers() const;                          \
  ModelObjectsTemp do_get_inputs() const {                              \
    ModelObjects ret;                                                   \
    ret+=get_input_containers();                                        \
    ret+=get_input_particles();                                         \
    return ret;                                                         \
  }                                                                     \
  ModelObjectsTemp do_get_outputs() const {                             \
    return ModelObjectsTemp();                                          \
  }                                                                     \
  void do_before_evaluate();                                            \
  ParticleIndexQuads get_all_possible_indexes() const;                     \
  IMP_OBJECT(Name)


#endif  /* IMPKERNEL_INTERNAL_LIST_LIKE_QUAD_CONTAINER_H */
