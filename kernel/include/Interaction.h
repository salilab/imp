/**
 *  \file Interaction.h
 *  \brief A shared base class to for classes which create interactions.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERACTION_H
#define IMP_INTERACTION_H

#include "Particle.h"

IMP_BEGIN_NAMESPACE


//! Sets of Particles
/** This is primarily used for representing lists of interacting particles. The
    particles in the list are not reference counted.*/
typedef std::vector<ParticlesTemp> ParticlesList;

//! A common base class for classes which create intra-particle interactions.
/** It is often important to know which particles interact with which
 other particles during optimization. These interactions come from
 a variety of different sources. In order to discover them, classes
 which create such interactions should inherit from this and implement
 the get_interacting_particles() method. This method, which can be expensive
returns the sets of interacting particles induced by the object.*/
class Interaction: public Object
{
 public:
  //! Return a list of sets of particles that are restrained by this restraint
  /** This function returns a list of sets of particles that are
      interacting within this restraint. Particles can appear in more
      than one set. However, if two particles never appear in the same
      set, then changing one of the particles should not change the
      derivatives of the other particle.

      It is always correct, if not optimal, to return
      \c ParticlesList(1, get_used_particles()).
      That is, one set, containing all the used particles.
   */
  virtual ParticlesList get_interacting_particles() const=0;

  /** Return the list of particles that this object depends on. This should
      include all particles whose attributes are read or written during
      the update calls.*/
  virtual ParticlesTemp get_used_particles() const =0;
};

#ifndef IMP_DOXYGEN
inline
ParticlesTemp get_union(const ParticlesList &pl) {
  ParticlesTemp ret=pl[0];
  for (unsigned int i=1; i < pl.size(); ++i) {
    ret.insert(ret.end(), pl[i].begin(), pl[i].end());
  }
  return ret;
}
#endif

IMP_END_NAMESPACE

#endif  /* IMP_INTERACTION_H */
