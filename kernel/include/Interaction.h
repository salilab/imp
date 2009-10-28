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
{protected:
  Interaction(std::string name=""): Object(name){}
 public:

  //! Return a list of sets of particles that are restrained by this restraint
  /** This function returns a list of sets of particles that are
      interacting within this restraint. Particles can appear in more
      than one set. However, if two particles never appear in the same
      set, then changing one of the particles should not change the
      derivatives of the other particle.

      This function should only be called when the objects in
      get_input_objects() below are all up to date.

      It is always correct, if not optimal, to return
      the list containing one set with all read and written particles
      in it.
   */
  virtual ParticlesList get_interacting_particles() const=0;


  /** \name Dependencies

      The set of run-time dependencies between Restaint, ScoreState,
      Particle, SingletonContainer and PairContainer objects is
      determined in two passes. The first, using functions called
      get_input_objects() and get_ouput_objects() is used to determine
      which objects need to be up to date in order to accurately
      determine the list of used particles. The second pass,
      get_input_particles(), get_output_particles() returns the set of
      particles which are used.

      For example, a restraint which stores a list of particles and only
      ever uses them should return
      - an empty list for get_output_particles(), get_input_objects()
      and get_output_objects()
      - the list of stored particles for get_input_particles()

      A Restraint which gets its particles from a SingletonContainer,
      and applies a refiner to each such particle should return
      - the SingletonContainer for get_input_objects()
      - an empty list for get_output_objects() and get_output_particles()
      - and the set of particles in the SingletonContainer, plus the
      results of Refiner::get_refined() apply to each of them for
      get_input_particles()

      A ScoreState which updates the contents of a PairContainer based
      on the contents of a SingletonContainer should return
      - the SingletonContainer for get_input_objects()
      - the PairContainer for get_output_objects()
      - the contents of the input SingletonContainer for get_input_particles()
      - an empty list for get_output_particles()

      A SingletonContainer which is the union of a set of
      other SingletonContainers should return
      - the set of input SingletonContainers for get_input_objects()
      - an empty list for get_output_objects()
      - and the empty list for get_input_particles() and get_output_particels
      (since it doesn't care about the contents of the particles).

      A ScoreState which update a particle based on the contents of some
      other particles:
      - empty lists for get_input_objects() and get_output_objects()
      - the input particles for get_input_particles()
      - and the output particle for get_output_particles()
      @{
  */
  virtual ObjectsTemp get_input_objects() const =0;

  virtual ObjectsTemp get_output_objects() const =0;

  virtual ParticlesTemp get_input_particles() const =0;

  virtual ParticlesTemp get_output_particles() const =0;
  /** @} */
};

IMP_END_NAMESPACE

#endif  /* IMP_INTERACTION_H */
