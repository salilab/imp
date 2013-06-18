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
class LogPairScore : public PairScore {
  mutable base::map<ParticlePair, unsigned int> map_;

 public:
  //! create with an empty map
  LogPairScore() {}
  virtual double evaluate_index(Model *m, const ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const ParticleIndexes &pis) const
    IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  IMP_PAIR_SCORE_METHODS(LogPairScore);
  IMP_OBJECT_METHODS(LogPairScore);

  //! Get a list of all pairs (without multiplicity)
  ParticlePairsTemp get_particle_pairs() const {
    ParticlePairsTemp ret;
    for (base::map<ParticlePair, unsigned int>::const_iterator it =
             map_.begin();
         it != map_.end(); ++it) {
      ret.push_back(it->first);
    }
    return ret;
  }
  //! Clear the lst of pairs
  void clear() { map_.clear(); }
  //! Return true if the pair is in the list
  bool get_contains(const ParticlePair &pp) const {
    return map_.find(pp) != map_.end();
  }
};

// doxygen spits out warnings
#ifndef IMP_DOXYGEN

inline Float LogPairScore::evaluate_index(Model *m, const ParticleIndexPair &p,
                                          DerivativeAccumulator *) const {
  ParticlePair pp(m->get_particle(p[0]), m->get_particle(p[1]));
  if (map_.find(pp) == map_.end()) {
    map_[pp] = 0;
  }
  ++map_[pp];
  return 0.;
}
#endif

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_LOG_PAIR_SCORE_H */
