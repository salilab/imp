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
#define IMP_SF_SET_ONLY(mask, particles, containers)            \
  {                                                             \
    ParticlesTemp cur=particles;                                \
    ContainersTemp ccur=containers;                             \
    for (unsigned int i=0; i<ccur.size(); ++i) {                \
      base::Object *po= ccur[i];                                \
      Particle *p= dynamic_cast<Particle*>(po);                 \
      if (p) cur.push_back(p);                                  \
    }                                                           \
    mask.reset();                                               \
    for (unsigned int i=0; i< cur.size(); ++i) {                \
      mask.set(get_as_unsigned_int(cur[i]->get_index()));       \
    }                                                           \
  }

#define IMP_SF_SET_ONLY_2(mask, particles, containers,                  \
                   particlestwo, containerstwo)                         \
  {                                                                     \
    ParticlesTemp curout=particles;                                     \
    ContainersTemp ccurout=containers;                                  \
    ParticlesTemp tcurout=particlestwo;                                 \
    ContainersTemp tccurout=containerstwo;                              \
    curout.insert(curout.end(), tcurout.begin(), tcurout.end());        \
    ccurout.insert(ccurout.end(), tccurout.begin(), tccurout.end());    \
    IMP_SF_SET_ONLY(mask, curout, ccurout);                             \
  }
#else
#define IMP_SF_SET_ONLY(mask, particles, containers)

#define IMP_SF_SET_ONLY_2(mask, particles, containers,  \
                   particlestwo, containerstwo)
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

template <class T, int V>
struct SFSetIt {
  T *t_;
  SFSetIt(T *t): t_(t){}
  ~SFSetIt() {
    *t_= T(V);
  }
};


IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_EVALUATE_UTILITY_H */
