/**
 *  \file TableParticleRefiner.h
 *  \brief A lookup based particle refiner
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_TABLE_PARTICLE_REFINER_H
#define IMPCORE_TABLE_PARTICLE_REFINER_H

#include "config.h"

#include "internal/core_version_info.h"
#include <IMP/ParticleRefiner.h>
#include <map>

IMPCORE_BEGIN_NAMESPACE

//! A lookup based particle refiner
/** Each particle is refined by returning the list
    of particles stored for it in a table.
 */
class IMPCOREEXPORT TableParticleRefiner: public ParticleRefiner
{
  std::map<Particle*, Particles> map_;
public:
  //! Get the individual particles from the passed SingletonContainer
  TableParticleRefiner();

  virtual ~TableParticleRefiner();

  //! Add a mapping to the table
  void add_particle(Particle *p, const Particles &ps);

  //! Remove a mapping from the table
  void remove_particle(Particle *p);

  //! Set the mapping for a particular particle
  void set_particle(Particle *p, const Particles &ps);

  IMP_PARTICLE_REFINER(internal::core_version_info)
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_TABLE_PARTICLE_REFINER_H */
