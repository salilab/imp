/**
 *  \file LogPairScore.h
 *  \brief Track the particles pairs passed to the pair score.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
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
  IMP_SIMPLE_PAIR_SCORE(LogPairScore, get_module_version_info());

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

inline Float LogPairScore::evaluate(const ParticlePair &pp,
                                    DerivativeAccumulator *) const {
  if (map_.find(pp) == map_.end()) {
    map_[pp]=0;
  }
  ++map_[pp];
  return 0.;
}

inline ParticlesList
LogPairScore::get_interacting_particles(const ParticlePair &p) const {
  return ParticlesList(1, get_input_particles(p));
}

inline ParticlesTemp
LogPairScore::get_input_particles(const ParticlePair &p) const {
  ParticlesTemp t(2);
  t[0]= p[0];
  t[1]= p[1];
  return t;
}


inline ContainersTemp
LogPairScore::get_input_containers(const ParticlePair &p) const {
  return ContainersTemp();
}




IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_LOG_PAIR_SCORE_H */
