/**
 *  \file SingletonListRestraint.h
 *  \brief Apply a SingletonScore to each particle in a list.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMPCORE_SINGLETON_LIST_RESTRAINT_H
#define __IMPCORE_SINGLETON_LIST_RESTRAINT_H

#include "core_exports.h"
#include "internal/core_version_info.h"

#include <IMP/Restraint.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonScore.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a SingletonScore to each Particle in a list.
/** \ingroup restraint
 */
class IMPCOREEXPORT SingletonListRestraint : public Restraint
{
public:
  //! Create the list restraint.
  /** \param[in] ss The function to apply to each particle.
      \param[in] ps The list of particles to use in the restraint.
   */
  SingletonListRestraint(SingletonScore *ss,
                         const Particles &ps=Particles());

  IMP_RESTRAINT(internal::core_version_info)

  using Restraint::add_particles;
  using Restraint::add_particle;
  using Restraint::clear_particles;
  using Restraint::set_particles;

  virtual ParticlesList get_interacting_particles() const;

protected:
  Pointer<SingletonScore> ss_;
};

IMPCORE_END_NAMESPACE

#endif  /* __IMPCORE_SINGLETON_LIST_RESTRAINT_H */
