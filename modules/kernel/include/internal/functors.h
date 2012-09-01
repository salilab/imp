/**
 *  \file IMP/functor.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_FUNCTORS_H
#define IMPKERNEL_INTERNAL_FUNCTORS_H

#include "../kernel_config.h"
#include <IMP/base/Pointer.h>
#include "../Model.h"

IMP_BEGIN_INTERNAL_NAMESPACE

template <class Pred, bool Sense>
class PredicateEquals {
  base::OwnerPointer<const Pred> p_;
  base::Pointer<Model> m_;
  int v_;
 public:
  typedef typename Pred::IndexArgument argument_type;
  typedef bool result_type;
  PredicateEquals(const Pred *p,
                  Model *m, int v): p_(p), m_(m), v_(v){}
  bool operator()(const argument_type &t) const {
    if (Sense) {
      return p_->get_value_index(m_, t)==v_;
    } else {
      return p_->get_value_index(m_, t)!=v_;
    }
  }
};


template <class Score>
class ScoreAccumulator {
  Model *m_;
  const Score *s_;
  DerivativeAccumulator *da_;
  double score_;
public:
  ScoreAccumulator(Model *m, const Score *s,
                   DerivativeAccumulator *da): m_(m), s_(s), da_(da),
                                               score_(0){}
  template <class Arg>
  void operator()(const Arg &a) {
    score_+= s_->Score::evaluate_index(m_, a, da_);
  }
  template <class C>
  void operator()(const base::Vector<C> &vect) {
    score_=std::for_each(vect.begin(), vect.end(), *this).get_score();
  }
  double get_score() const {return score_;}
};

template <class Score>
class ScoreAccumulatorIfGood {
  Model *m_;
  const Score *s_;
  double max_;
  DerivativeAccumulator *da_;
  double score_;
public:
  ScoreAccumulatorIfGood(Model *m, const Score *s, double max,
                         DerivativeAccumulator *da): m_(m), s_(s),
                                                     max_(max), da_(da),
                                                     score_(0){}
  template <class Arg>
  void operator()(const Arg &a) {
    double cur= s_->Score::evaluate_index(m_, a, da_);
    score_+=cur;
    max_-=cur;
  }
  template <class C>
  void operator()(const base::Vector<C> &vect) {
    for (unsigned int i=0; i< vect.size(); ++i) {
      operator()(vect[i]);
      if (max_<0) break;
    }
  }
  double get_score() const {return score_;}
};

template <class Modifier>
class ModifierDerivativeApplier {
  Model *m_;
  const Modifier *s_;
  DerivativeAccumulator da_;
public:
  ModifierDerivativeApplier(Model *m, const Modifier *s,
                            DerivativeAccumulator da): m_(m), s_(s), da_(da){}
  template <class Arg>
  void operator()(const Arg &a) {
    s_->Modifier::apply_index(m_, a, da_);
  }
  template <class C>
  void operator()(const base::Vector<C> &vect) {
    std::for_each(vect.begin(), vect.end(), *this);
  }
};

template <class Modifier>
class ModifierApplier {
  Model *m_;
  const Modifier *s_;
public:
  ModifierApplier(Model *m, const Modifier *s): m_(m), s_(s){}
  template <class Arg>
  void operator()(const Arg &a) {
    s_->Modifier::apply_index(m_, a);
  }
  template <class C>
  void operator()(const base::Vector<C> &vect) {
    std::for_each(vect.begin(), vect.end(), *this);
  }
};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_FUNCTORS_H */
