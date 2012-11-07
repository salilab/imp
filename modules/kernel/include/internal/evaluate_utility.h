/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_EVALUATE_UTILITY_H
#define IMPKERNEL_INTERNAL_EVALUATE_UTILITY_H

#include "../kernel_config.h"
#include "../Particle.h"
#include <boost/dynamic_bitset.hpp>
#if IMP_BUILD < IMP_FAST
#define IMP_SF_SET_ONLY(mask, inputs)                           \
  {                                                             \
    ParticlesTemp cur=IMP::get_input_particles(inputs);         \
    mask.reset();                                               \
    for (unsigned int i=0; i< cur.size(); ++i) {                \
      mask.set(get_as_unsigned_int(cur[i]->get_index()));       \
    }                                                           \
  }

#define IMP_SF_SET_ONLY_2(mask, inputs1, inputs2)                       \
  {                                                                     \
    ModelObjectsTemp inputs_cur=inputs1;                                \
    inputs_cur+=inputs2;                                                \
    IMP_SF_SET_ONLY(mask, inputs_cur);                                  \
  }
#else
#define IMP_SF_SET_ONLY(mask, inputs)

#define IMP_SF_SET_ONLY_2(mask, inputs1, inputs2)
#endif

IMP_BEGIN_INTERNAL_NAMESPACE
struct SFResetBitset {
  boost::dynamic_bitset<> &bs_;
  bool val_;
  SFResetBitset(boost::dynamic_bitset<> &bs,
              bool val): bs_(bs), val_(val){}
  ~SFResetBitset() {
    if (val_) {
      bs_.set();
    } else {
      bs_.reset();
    }
  }
};

template <class T>
struct SFSetIt {
  T *t_;
  T old_;
  SFSetIt(T *t, T nv): t_(t), old_(*t){
    *t_=nv;
  }
  ~SFSetIt() {
    *t_= old_;
  }
};



IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_EVALUATE_UTILITY_H */
