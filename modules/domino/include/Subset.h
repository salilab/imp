/**
 *  \file IMP/domino/Subset.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_SUBSET_H
#define IMPDOMINO_SUBSET_H

#include <IMP/domino/domino_config.h>
#include "IMP/macros.h"
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/Value.h>
#include <IMP/base/ConstVector.h>
#include <algorithm>
#include <IMP/base/hash.h>

IMPDOMINO_BEGIN_NAMESPACE

//! Represent a subset of the particles being optimized.
/** Domino acts by dividing the particles being changed
    into subsets and optimizing the subsets independently.
    Each subset is represented using a Subset class. These
    classes, like the Assignment classes simply store
    a constant list (in this case of particles). The list
    is stored in sorted order. Their interface is more or
    less that of a constant vector in C++ or
    a constant list in python.
 */
class IMPDOMINOEXPORT Subset:
  public base::ConstVector<base::WeakPointer<Particle>,
                          Particle*> {
  typedef base::ConstVector<base::WeakPointer<Particle>, Particle* > P;
  static const ParticlesTemp &get_sorted(ParticlesTemp &ps) {
    std::sort(ps.begin(), ps.end());
    return ps;
  }
 public:
#ifndef IMP_DOXYGEN
  // only use this if particles are sorted already
  Subset(const ParticlesTemp &ps, bool):
      P(ps.begin(), ps.end()) {
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (unsigned int i=0; i< size(); ++i) {
        IMP_CHECK_OBJECT(ps[i]);
      }
      for (unsigned int i=1; i< ps.size(); ++i) {
        IMP_INTERNAL_CHECK(ps[i-1] < ps[i], "Particles not ordered");
      }
    }
  }
#endif
  Subset(){}
  /** Construct a subset from a non-empty list of particles.
   */
  explicit Subset(ParticlesTemp ps): P(get_sorted(ps)) {
    IMP_USAGE_CHECK(!ps.empty(), "Do not create empty subsets");
    IMP_IF_CHECK(USAGE) {
      std::sort(ps.begin(), ps.end());
      IMP_USAGE_CHECK(std::unique(ps.begin(), ps.end()) == ps.end(),
                      "Duplicate particles in set");
      for (unsigned int i=0; i< ps.size(); ++i) {
        IMP_CHECK_OBJECT(ps[i]);
      }
    }
  }
  Model *get_model() const {
    return operator[](0)->get_model();
  }
  std::string get_name() const;
  bool get_contains(const Subset &o) const {
    return std::includes(begin(), end(), o.begin(), o.end());
  }
};

IMP_VALUES(Subset, Subsets);
IMP_SWAP(Subset);

inline
Subset get_union(const Subset &a, const Subset &b) {
  ParticlesTemp pt;
  set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(pt));
  return Subset(pt, true);
}

inline
Subset get_intersection(const Subset &a, const Subset &b) {
  ParticlesTemp pt;
  set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                   std::back_inserter(pt));
  if (pt.empty()) {
    return Subset();
  } else {
    return Subset(pt, true);
  }
}


inline Subset get_difference(const Subset &a, const Subset &b) {
  ParticlesTemp rs;
  std::set_difference(a.begin(), a.end(),
                      b.begin(), b.end(),
                      std::back_inserter(rs));
  Subset ret(rs, true);
  return ret;
}

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SUBSET_H */
