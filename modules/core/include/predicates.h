/**
 *  \file IMP/core/predicates.h
 *  \brief Score particles based on a bounding box
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_PREDICATES_H
#define IMPCORE_PREDICATES_H

#include <IMP/core/core_config.h>
#include <IMP/SingletonPredicate.h>
#include <IMP/singleton_macros.h>
#include "XYZ.h"

IMPCORE_BEGIN_NAMESPACE
/** Return 1 if the XYZ is in the bounding box, 0 otherwise.*/
class InBoundingBox3DSingletonPredicate : public SingletonPredicate {
  algebra::BoundingBox3D bb_;

 public:
  InBoundingBox3DSingletonPredicate(const algebra::BoundingBox3D &bb,
                                    std::string name =
                                        "InBoundingBox3DSingletonPredicate%1%")
      : SingletonPredicate(name), bb_(bb) {}
  IMP_INDEX_SINGLETON_PREDICATE(
      InBoundingBox3DSingletonPredicate,
      return bb_.get_contains(XYZ(m, pi).get_coordinates()) ? 1 : 0;
      , {
    ModelObjectsTemp ret;
    ret += IMP::get_particles(m, pi);
    return ret;
  });

};
/** Return the value of an int attribute as the predicate value.*/
class AttributeSingletonPredicate : public SingletonPredicate {
  IntKey bb_;

 public:
  AttributeSingletonPredicate(IntKey bb, std::string name =
                                             "AttributeSingletonPredicate%1%")
      : SingletonPredicate(name), bb_(bb) {}
  IMP_INDEX_SINGLETON_PREDICATE(AttributeSingletonPredicate,
                                return m->get_attribute(bb_, pi), {
    ModelObjectsTemp ret;
    ret += IMP::get_particles(m, pi);
    return ret;
  });
};

/** Use a predicate to determine which score to apply. One can use this to,
    for example,  truncate a score using a bounding box.*/
template <class Predicate, class Score = SingletonScore>
class PredicateSingletonScore : public SingletonScore {
  OwnerPointer<Predicate> pred_;
  base::Vector<OwnerPointer<Score> > scores_;
  int offset_;
  Score *get_score(int val) const {
    if (val < offset_ || val > scores_.size() + offset_) {
      return nullptr;
    } else {
      return scores_[val + offset_];
    }
  }

 public:
  PredicateSingletonScore(Predicate *pred,
                          std::string name = "PredicateScore%1%")
      : SingletonScore(name), pred_(pred), offset_(0) {}
  void set_singleton_score(int val, Score *score) {
    IMP_USAGE_CHECK(val >= offset_,
                    "Negative predicate values not supported yet");
    scores_.resize(std::max<int>(val + 1, scores_.size()));
    scores_[val] = score;
  }
  IMP_INDEX_SINGLETON_SCORE(PredicateSingletonScore);
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
template <class Predicate, class Score>
inline double PredicateSingletonScore<Predicate, Score>::evaluate_index(
    Model *m, ParticleIndex p, DerivativeAccumulator *da) const {
  int val = pred_->get_value(m, p);
  Score *s = get_score(val);
  if (s) {
    return s->Score::evaluate_index(m, p, da);
  } else {
    return 0;
  }
}
template <class Predicate, class Score>
inline void PredicateSingletonScore<Predicate, Score>::do_show(
    std::ostream &) const {}
#endif

/** Other overloads can be created as needed.*/
template <class Predicate, class Score>
inline PredicateSingletonScore<Predicate, Score> *
create_predicates_singleton_score(Predicate *pred, int val, Score *score) {
  Pointer<PredicateSingletonScore<Predicate, Score> > ret =
      new PredicateSingletonScore<Predicate, Score>(pred);
  ret->set_score(val, score);
  return ret;
}

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_PREDICATES_H */
