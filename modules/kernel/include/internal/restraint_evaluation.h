/**
 *  \file interna/constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H
#define IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H

#include "../kernel_config.h"
#include <boost/dynamic_bitset.hpp>
#include "../Key.h"
#include "../utility.h"
#include <IMP/base/exception.h>
#include <IMP/base/log.h>
#include <IMP/algebra/Sphere3D.h>
#include <boost/scoped_ptr.hpp>


#if IMP_BUILD < IMP_FAST
#define IMP_SET_ONLY(mask, particles, containers)               \
  {                                                             \
    ParticlesTemp cur=particles;                                \
    ContainersTemp ccur=containers;                             \
    for (unsigned int i=0; i<ccur.size(); ++i) {                \
      Object *po= ccur[i];                                      \
      Particle *p= dynamic_cast<Particle*>(po);                 \
      if (p) cur.push_back(p);                                  \
    }                                                           \
    mask.reset();                                               \
    for (unsigned int i=0; i< cur.size(); ++i) {                \
      mask.set(cur[i]->get_index());                            \
    }                                                           \
  }

#define IMP_SET_ONLY_2(mask, particles, containers,                     \
                   particlestwo, containerstwo)                         \
  {                                                                     \
    ParticlesTemp curout=particles;                                     \
    ContainersTemp ccurout=containers;                                  \
    ParticlesTemp tcurout=particlestwo;                                 \
    ContainersTemp tccurout=containerstwo;                              \
    curout.insert(curout.end(), tcurout.begin(), tcurout.end());        \
    ccurout.insert(ccurout.end(), tccurout.begin(), tccurout.end());    \
    IMP_SET_ONLY(mask, curout, ccurout);                                \
  }



#define IMP_WRAP_EVALUATE_CALL(restraint, expr, m)                      \
  if (m->first_call_) {                                                 \
    ResetBitset rbr(m->Masks::read_mask_, true);                        \
    ResetBitset rbw(m->Masks::write_mask_, true);                       \
    ResetBitset rbar(m->Masks::add_remove_mask_, true);                 \
    ResetBitset rbrd(m->Masks::read_derivatives_mask_, true);           \
    ResetBitset rbwd(m->Masks::write_derivatives_mask_, true);          \
    m->Masks::write_mask_.reset();                                      \
    m->Masks::add_remove_mask_.reset();                                 \
    m->Masks::read_derivatives_mask_.reset();                           \
    IMP_SET_ONLY(m->Masks::read_mask_, restraint->get_input_particles(), \
                 restraint->get_input_containers()                      \
                 );                                                     \
    IMP_SET_ONLY(m->Masks::write_derivatives_mask_,                     \
                 restraint->get_input_particles(),                      \
                 restraint->get_input_containers()                      \
                 );                                                     \
    IMP_SET_ONLY(m->Masks::read_derivatives_mask_,                      \
            restraint->get_input_particles(),                           \
             restraint->get_input_containers()                          \
             );                                                         \
    IMP_CHECK_OBJECT(restraint);                                        \
    expr;                                                               \
  } else {                                                              \
    IMP_CHECK_OBJECT(restraint);                                        \
    expr;                                                               \
  }                                                                     \

#else
#define IMP_WRAP_EVALUATE_CALL(restraint, expr, m) expr
#endif


IMP_BEGIN_INTERNAL_NAMESPACE

template <class T, int V>
struct SetIt {
  T *t_;
  SetIt(T *t): t_(t){}
  ~SetIt() {
    *t_= T(V);
  }
};

struct ResetBitset {
  boost::dynamic_bitset<> &bs_;
  bool val_;
  ResetBitset(boost::dynamic_bitset<> &bs,
              bool val): bs_(bs), val_(val){}
  ~ResetBitset() {
    if (val_) {
      bs_.set();
    } else {
      bs_.reset();
    }
  }
};

struct ModelWeights {
  double operator()(const Restraint *r, unsigned int ) const {
    return r->model_weight_;
  }
};
struct ExternalWeights {
  const Floats& w_;
  ExternalWeights(const Floats &w): w_(w){}
  double operator()(const Restraint *, unsigned int i) const {
    return w_[i];
  }
};


template <bool DERIV, bool GOOD, bool MAX, bool STATS, class RS, class Weights>
inline Floats evaluate_restraints(const RS &restraints,
                                  const Weights &weights, double omax,
                                  Model *m) {
  IMP_FUNCTION_LOG;
  IMP_INTERNAL_CHECK(!GOOD || !MAX, "Can't be both max and good");
  Floats ret;
  double remaining=omax;
  if (!MAX) {
    remaining=m->get_maximum_score();
  }
  boost::scoped_ptr<boost::timer> timer;
  if (STATS) timer.reset(new boost::timer());
  const unsigned int rsz=restraints.size();
  for (unsigned int i=0; i< rsz; ++i) {
    double value=0;
    double weight=weights(restraints[i], i);
    IMP_USAGE_CHECK(weight>=0, "Weight was not initialized for \""
                    << restraints[i]->get_name() << '"');
    DerivativeAccumulator accum(weight);
    if (STATS) timer->restart();
    if (GOOD) {
      double max=std::min(remaining,
                          restraints[i]->get_maximum_score());
      IMP_WRAP_EVALUATE_CALL(restraints[i],
                         value=
                   restraints[i]->unprotected_evaluate_if_good(DERIV?
                                                              &accum:nullptr,
                                                               max),
                             m);
    } else if (MAX) {
      IMP_WRAP_EVALUATE_CALL(restraints[i],
                         value=
                   restraints[i]->unprotected_evaluate_if_good(DERIV?
                                                              &accum:nullptr,
                                                               remaining),
                             m);
    } else {
      IMP_WRAP_EVALUATE_CALL(restraints[i],
                         value=
                         restraints[i]->unprotected_evaluate(DERIV?
                                                             &accum:nullptr),
                             m);
    }
    double wvalue= weight*value;
    remaining-=wvalue;
    IMP_LOG(TERSE, restraints[i]->get_name()<<  " score is "
            << wvalue << " (" << weight << ")" << std::endl);
    if (m->gather_statistics_) {
      m->add_to_restraint_evaluate(restraints[i], timer->elapsed(), wvalue);
    }
    if (value > restraints[i]->get_maximum_score()) {
      m->has_good_score_=false;
    }
    ret.push_back(wvalue);
  }
  if (remaining<0) {
    m->has_good_score_=false;
  }
  return ret;
}

template < bool DERIV, bool GOOD, bool MAX, bool STATS,
          class Weights, class SS, class RS>
inline Floats evaluate(const SS &states,
                       const RS &restraints,
                       const Weights &weights, double omax,
                       Model *m) {
  IMP_CHECK_OBJECT(m);
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< restraints.size(); ++i) {
      IMP_CHECK_CODE(Restraint *r= restraints[i]);
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(r),
                      "Cannot pass RestraintSets to Model::evaluate().");
    }
  }
  m->has_good_score_=true;
  // make sure stage is restored on an exception
  SetIt<IMP::internal::Stage, internal::NOT_EVALUATING> reset(&m->cur_stage_);
  IMP_CHECK_OBJECT(m);
  IMP_LOG(VERBOSE, "On restraints " << restraints
          << " and score states " << states
          << std::endl);
  m->before_evaluate(get_as<ScoreStatesTemp>(states));

  m->cur_stage_= internal::EVALUATING;
  if (DERIV) {
    m->zero_derivatives();
  }
  Floats ret= evaluate_restraints<DERIV, GOOD, MAX, STATS>(restraints,
                                                           weights,omax,
                                                           m);

  m->after_evaluate(get_as<ScoreStatesTemp>(states), DERIV);

  // validate derivatives
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    if (DERIV) {
      m->validate_computed_derivatives();
    }
  }
  IMP_LOG(TERSE, "Final score: "
          << std::accumulate(ret.begin(), ret.end(), 0.0) << std::endl);
  ++m->eval_count_;
  m->first_call_=false;
  return ret;
}


IMP_END_INTERNAL_NAMESPACE


#endif  /* IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H */
