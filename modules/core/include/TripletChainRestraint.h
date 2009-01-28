/**
 *  \file TripletChainRestraint.h
 *   \brief Restraint triplets of particles in chains.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_TRIPLET_CHAIN_RESTRAINT_H
#define IMPCORE_TRIPLET_CHAIN_RESTRAINT_H

#include "config.h"
#include "internal/core_version_info.h"

#include <IMP/Restraint.h>
#include <IMP/Pointer.h>
#include <IMP/TripletScore.h>
#include <IMP/container_macros.h>

#include <vector>

IMPCORE_BEGIN_NAMESPACE

//! Restrain each triplet of consecutive particles in each chain.
/** \deprecated Use a TripletContainer instead
 */
class IMPCOREEXPORT TripletChainRestraint : public Restraint
{
public:
  //! Create the triplet restraint.
  /** \param[in] trip_score Triplet score to apply.
   */
  TripletChainRestraint(TripletScore* trip_score);
  virtual ~TripletChainRestraint(){}

  virtual ParticlesList get_interacting_particles() const;

  IMP_RESTRAINT(internal::core_version_info)

  //! Add a chain of particles
  /** Each three successive particles are restrained.
   */
  void add_chain(const Particles &ps);

  //! Clear all the stored chains
  void clear_chains();

  IMP_LIST(private, Particle, particle, Particle*);
protected:
  Pointer<TripletScore> ts_;
  std::vector<unsigned int> chain_splits_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_TRIPLET_CHAIN_RESTRAINT_H */
