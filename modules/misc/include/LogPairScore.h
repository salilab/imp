/**
 *  \file LogPairScore.h
 *  \brief Track the particles pairs passed to the pair score.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_LOG_PAIR_SCORE_H
#define IMPMISC_LOG_PAIR_SCORE_H

#include "config.h"
#include "internal/version_info.h"
#include <IMP/PairScore.h>
#include <map>

IMPMISC_BEGIN_NAMESPACE

//! Track the pairs of particles passed.
/** Primarily for testing.
 */
class LogPairScore : public PairScore
{
  mutable std::map<ParticlePair, unsigned int> map_;
 public:
  //! create with an empty map
  LogPairScore(){}
  virtual ~LogPairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *) const {
    ParticlePair pp(a,b);
    if (map_.find(pp) == map_.end()) {
      map_[pp]=0;
    }
    ++map_[pp];
    return 0.;
  }
  //! Get a list of all pairs (without multiplicity)
  ParticlePairs get_particle_pairs() const {
    ParticlePairs ret;
    for (std::map<ParticlePair, unsigned int>::const_iterator
           it = map_.begin(); it != map_.end(); ++it) {
      ret.push_back(it->first);
    }
    return ret;
  }
  //! Clear the lst of pairs
  void clear() {
    map_.clear();
  }
  virtual void show(std::ostream &out=std::cout) const {
    out << "LogPS"<<std::endl;
  }
  //! Return true if the pair is in the list
  bool get_contains(const ParticlePair &pp) const {
    return map_.find(pp) != map_.end();
  }
  VersionInfo get_version_info() const {
    return internal::version_info;
  }
};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_LOG_PAIR_SCORE_H */
