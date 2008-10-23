/**
 *  \file ConnectivityRestraint.h    \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_CONNECTIVITY_RESTRAINT_H
#define IMPCORE_CONNECTIVITY_RESTRAINT_H

#include "core_exports.h"
#include "internal/core_version_info.h"
#include "DistanceRestraint.h"

#include <IMP/Restraint.h>
#include <IMP/PairScore.h>

IMPCORE_BEGIN_NAMESPACE

//! Ensure that a set of particles remains connected with one another.
/** The restraint takes several particles and ensures that they remain
    connected. If you wish to restraint the connectivity of sets of
    particles (i.e. each protein is represented using a set of balls)
    use an appropriate PairScore which calls a ParticleRefiner (such
    as ClosestPairPairScore).

    More precisely, the restraint scores based on a minimum spanning
    tree on the points defined by the particles.

    \ingroup restraint
 */
class IMPCOREEXPORT ConnectivityRestraint : public Restraint
{
public:
  ConnectivityRestraint(PairScore* ps);

  //!
  using Restraint::add_particle;
  //!
  using Restraint::add_particles;
  //!
  using Restraint::clear_particles;
  //!
  using Restraint::set_particles;

  virtual ~ConnectivityRestraint();

  IMP_RESTRAINT(internal::core_version_info)

protected:

    Pointer<PairScore> ps_;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CONNECTIVITY_RESTRAINT_H */
