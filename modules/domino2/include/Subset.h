/**
 *  \file domino2/Subset.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_SUBSET_H
#define IMPDOMINO2_SUBSET_H

#include "domino2_config.h"
#include "IMP/macros.h"
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/Pointer.h>
#include <boost/shared_array.hpp>
#include <boost/functional/hash.hpp>
#include <algorithm>
#include <boost/functional/hash/hash.hpp>

IMPDOMINO2_BEGIN_NAMESPACE

//! Represent a subset of the particles being optimized.
/** Domino acts by dividing the particles being changed
    into subsets and optimizing the subsets independently.
    Each subset is represented using a Subset class. These
    classes, like the SubsetState classes simply store
    a constant list (in this case of particles). The list
    is stored in sorted order. Their interface is more or
    less that of a constant std::vector in C++ or
    a constant list in python.
 */
class IMPDOMINO2EXPORT Subset {
  boost::shared_array<Particle*> ps_;
  unsigned int sz_;
  int compare(const Subset &o) const {
    if (sz_ < o.sz_) return -1;
    else if (sz_ > o.sz_) return 1;
    for (unsigned int i=0; i< size(); ++i) {
      if (ps_[i] < o[i]) return -1;
      else if (ps_[i] > o[i]) return 1;
    }
    return 0;
  }
 public:
  typedef Subset This;
#ifndef IMP_DOXYGEN
  // only use this if particles are sorted already
  Subset(const ParticlesTemp &ps, bool):
    ps_(new Particle*[ps.size()]),
    sz_(ps.size()){
    std::copy(ps.begin(), ps.end(), ps_.get());
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (unsigned int i=0; i< sz_; ++i) {
        IMP_CHECK_OBJECT(ps_[i]);
      }
      for (unsigned int i=1; i< ps.size(); ++i) {
        IMP_INTERNAL_CHECK(ps[i-1] < ps[i], "Particles not ordered");
      }
    }
  }
#endif
  Subset(): sz_(0){}
  Subset(ParticlesTemp ps):
    ps_(new Particle*[ps.size()]),
    sz_(ps.size()) {
    std::sort(ps.begin(), ps.end());
    std::copy(ps.begin(), ps.end(), ps_.get());
    IMP_USAGE_CHECK(std::unique(ps.begin(), ps.end()) == ps.end(),
                    "Duplicate particles in set");
    IMP_IF_CHECK(USAGE) {
      for (unsigned int i=0; i< ps.size(); ++i) {
        IMP_CHECK_OBJECT(ps[i]);
      }
    }
  }
  unsigned int size() const {
    return sz_;
  }
  Model *get_model() const {
    return ps_[0]->get_model();
  }
#ifndef SWIG
  Particle *operator[](unsigned int i) const {
    IMP_USAGE_CHECK( i < sz_, "Out of range");
    return ps_[i];
  }
#ifndef IMP_DOXYGEN
  typedef Particle** const_iterator;
  const_iterator begin() const {
    return ps_.get();
  }
  const_iterator end() const {
    return ps_.get()+sz_;
  }
#else
  class const_iterator;
  const_iterator begin() const;
  const_iterator end() const;
#endif
#endif
#ifndef IMP_DOXYGEN
  Particle* __getitem__(unsigned int i) const {
    if (i >= sz_) IMP_THROW("Out of bound", IndexException);
    return ps_[i];}
  unsigned int __len__() const {return sz_;}
#endif
  IMP_SHOWABLE(Subset);
  std::string get_name() const;
  IMP_HASHABLE_INLINE(Subset, return boost::hash_range(begin(),
                                                       end()););
  IMP_COMPARISONS;
};

IMP_VALUES(Subset, Subsets);
IMP_OUTPUT_OPERATOR(Subset);

#ifndef IMP_DOXYGEN
inline std::size_t hash_value(const Subset &t) {
  return t.__hash__();
}
#endif

IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_H */
