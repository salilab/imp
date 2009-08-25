/**
 *  \file LogPairScore.h
 *  \brief Track the particles pairs passed to the pair score.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_LOG_PAIR_SCORE_H
#define IMPMISC_LOG_PAIR_SCORE_H

#include "config.h"
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
  IMP_PAIR_SCORE(LogPairScore, get_module_version_info());

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
  //! Return true if the pair is in the list
  bool get_contains(const ParticlePair &pp) const {
    return map_.find(pp) != map_.end();
  }
};

inline void LogPairScore::show(std::ostream &out) const {
  out << "LogPairScore";
}

inline Float LogPairScore::evaluate(Particle *a, Particle *b,
                             DerivativeAccumulator *) const {
  ParticlePair pp(a,b);
  if (map_.find(pp) == map_.end()) {
    map_[pp]=0;
  }
  ++map_[pp];
  return 0.;
}

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_LOG_PAIR_SCORE_H */
