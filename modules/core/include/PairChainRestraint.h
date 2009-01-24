/**
 *  \file PairChainRestraint.h
 *  \brief Restrain pairs of particles in chains.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_PAIR_CHAIN_RESTRAINT_H
#define IMPCORE_PAIR_CHAIN_RESTRAINT_H

#include "config.h"
#include "internal/core_version_info.h"

#include <IMP/Restraint.h>
#include <IMP/Pointer.h>
#include <IMP/PairScore.h>

#include <vector>

IMPCORE_BEGIN_NAMESPACE

//! This class is depreceated, use a ParticlePairsRestraint instead.
/** \deprecated Use a ParticlePairsRestraint instead.
 */
class IMPCOREEXPORT PairChainRestraint : public Restraint
{
public:
  //! Create the pair restraint.
  /** \param[in] pair_score Pair score to apply.
   */
  PairChainRestraint(PairScore* pair_score);
  virtual ~PairChainRestraint(){}

  IMP_RESTRAINT(internal::core_version_info)

  //! Add a chain of particles
  /** Each two successive particles are restrained.
   */
  void add_chain(const Particles &ps);

  //! Clear all the stored chains
  void clear_chains();

  virtual ParticlesList get_interacting_particles() const;

  IMP_LIST(public, Particle, particle, Particle*)

protected:
  Pointer<PairScore> ts_;
  std::vector<unsigned int> chain_splits_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_PAIR_CHAIN_RESTRAINT_H */
