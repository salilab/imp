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
#include <boost/scoped_array.hpp>
#include <algorithm>

IMPDOMINO2_BEGIN_NAMESPACE

/** Store a subset of the optimized particles. The particles are
    kept in sorted order. Subsets should be stored by pointer.
 */
class IMPDOMINO2EXPORT Subset: public RefCounted {
  const Particles ps_;
  static Particles get_sorted(const Particles &in) {
    Particles s(in);
    std::sort(s.begin(), s.end());
    return s;
  }
public:
#ifndef IMP_DOXYGEN
  Subset(const ParticlesTemp &ps, bool are_sorted): ps_(ps) {
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (unsigned int i=1; i< ps.size(); ++i) {
        IMP_INTERNAL_CHECK(ps[i-1] < ps[i], "Particles not ordered");
      }
    }
  }
#endif
  Subset(const ParticlesTemp &ps): ps_(get_sorted(ps)) {
  }
  unsigned int get_number_of_particles() const {
    return ps_.size();
  }
  Particle *get_particle(unsigned int i) const {
    return ps_[i];
  }
#ifndef IMP_DOXYGEN
  const Particles& get_particles() const {
    return ps_;
  }
#endif
  Model *get_model() const {
    return ps_[0]->get_model();
  }
#ifndef SWIG
#ifndef IMP_DOXYGEN
  typedef Particles::const_iterator ParticlesIterator;
  ParticlesIterator particles_begin() const {
    return ps_.begin();
  }
  ParticlesIterator particles_end() const {
    return ps_.end();
  }
#else
  class ParticlesIterator;
  ParticlesIterator particles_begin() const;
  ParticlesIterator particles_end() const;
#endif
#endif
  void show(std::ostream &out=std::cout) const;
  std::string get_name() const;
};

IMP_OBJECTS(Subset, Subsets);

IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_H */
