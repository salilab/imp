/**
 *  \file IMP/misc/LogPairScore.h
 *  \brief Track the particles pairs passed to the pair score.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_LOG_PAIR_SCORE_H
#define IMPMISC_LOG_PAIR_SCORE_H

#include <IMP/misc/misc_config.h>
#include <IMP/PairScore.h>
#include <IMP/generic.h>
#include <IMP/base/map.h>

IMPMISC_BEGIN_NAMESPACE

//! Track the pairs of particles passed.
/** Primarily for testing.
 */
class LogPairScore : public PairScore
{
  mutable base::map<ParticlePair, unsigned int> map_;
 public:
  //! create with an empty map
  LogPairScore(){}
  IMP_SIMPLE_PAIR_SCORE(LogPairScore);

  //! Get a list of all pairs (without multiplicity)
  ParticlePairsTemp get_particle_pairs() const {
    ParticlePairsTemp ret;
    for (base::map<ParticlePair, unsigned int>::const_iterator
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

// doxygen spits out warnings
#ifndef IMP_DOXYGEN
inline void LogPairScore::do_show(std::ostream &) const {
}

inline Float LogPairScore::evaluate(const ParticlePair &pp,
                                    DerivativeAccumulator *) const {
  if (map_.find(pp) == map_.end()) {
    map_[pp]=0;
  }
  ++map_[pp];
  return 0.;
}
#endif


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_LOG_PAIR_SCORE_H */
