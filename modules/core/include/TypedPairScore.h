/**
 *  \file IMP/core/TypedPairScore.h
 *  \brief Delegate to another PairScore depending on particle types.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_TYPED_PAIR_SCORE_H
#define IMPCORE_TYPED_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>

IMPCORE_BEGIN_NAMESPACE

//! Delegate to another PairScore depending on particle types.
/** Each particle is given an integer type, which is used at evaluate time
    to select a PairScore to use for a given pair of particles. (The ordering
    of the particles does not matter.) After creating the object, call
    set_pair_score() to tell it the PairScore objects you want to use for each
    pair of particle types.

    \note For efficiency, you should probably use the
    container::PredicatePairRestraint instead.
 */
class IMPCOREEXPORT TypedPairScore : public PairScore {
  // The key used for the particle types.
  IntKey typekey_;
  typedef std::map<std::pair<Int, Int>, IMP::PointerMember<PairScore> >
      ScoreMap;
  // Mapping from particle types to PairScores.
  ScoreMap score_map_;
  // Whether to throw an exception for invalid particle types.
  bool allow_invalid_types_;

  PairScore *get_pair_score(const ParticlePair &pp) const;

 public:
  //! Constructor.
  /** \param[in] typekey The IntKey used to denote the type of each particle.
      \param[in] allow_invalid_types Desired behavior for particle pairs that
                 have types not covered by set_pair_score(). If true, the score
                 returned for these pairs is zero. If false, evaluate() raises
                 a ValueException.
   */
  TypedPairScore(IntKey typekey, bool allow_invalid_types = true);

  //! Set the particle's type.
  /** At evaluate time, if a given particle does not have the typekey
      attribute, this method is called. Here it does nothing, but it could
      be overridden in a subclass to automatically set the type of a particle,
      e.g. from other particle attributes such as an atom or residue name.
   */
  virtual void set_particle_type(Particle *) const {}

  //! Set the PairScore to delegate to for a given pair of particle types.
  /** \param[in] ps PairScore to use at evaluate time.
      \param[in] atype First particle type.
      \param[in] btype Second particle type.
   */
  void set_pair_score(PairScore *ps, Int atype, Int btype) {
    score_map_
        [std::pair<Int, Int>(std::min(atype, btype), std::max(atype, btype))] =
            IMP::PointerMember<PairScore>(ps);
  }

  virtual double evaluate_index(Model *m,
                                const ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_PAIR_SCORE_METHODS(TypedPairScore);
  IMP_OBJECT_METHODS(TypedPairScore);
  ;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_TYPED_PAIR_SCORE_H */
