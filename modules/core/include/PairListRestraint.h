/**
 *  \file PairListRestraint.h
 *  \brief Apply a PairScore to each particle pair in a list.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMPCORE_PAIR_LIST_RESTRAINT_H
#define __IMPCORE_PAIR_LIST_RESTRAINT_H

#include "core_exports.h"
#include "internal/core_version_info.h"

#include <IMP/Restraint.h>
#include <IMP/Particle.h>
#include <IMP/Pointer.h>
#include <IMP/PairScore.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a PairScore to each pair of particles in a list.
/** \ingroup restraint
 */
class IMPCOREEXPORT PairListRestraint : public Restraint
{
public:
  //! Create the list restraint.
  /** \param[in] ss The function to apply to each particle.
      \param[in] ps The list of particle pairs to use in the restraints
   */
  PairListRestraint(PairScore *ss, const ParticlePairs &ps=ParticlePairs());
  virtual ~PairListRestraint();

  IMP_RESTRAINT(internal::core_version_info)

  void add_particle_pair(ParticlePair p);
  void clear_particle_pairs();
  void add_particle_pairs(const ParticlePairs &ps);

  virtual ParticlesList get_interacting_particles() const;
protected:
  Pointer<PairScore> ss_;
};

IMPCORE_END_NAMESPACE

#endif  /* __IMPCORE_PAIR_LIST_RESTRAINT_H */
