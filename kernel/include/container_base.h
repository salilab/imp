/**
 *  \file IMP/container_base.h
 *  \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_CONTAINER_BASE_H
#define IMP_CONTAINER_BASE_H

#include "config.h"
#include "Object.h"
#include "macros.h"

IMP_BEGIN_NAMESPACE

class Particle;
IMP_OBJECTS(Particle);
/** \objects{Particle}
*/
/** \objectstemp{Particle}
*/

class Container;
IMP_OBJECTS(Container);
/** \objects{Container}
*/
/** \objectstemp{Container}
*/

//! Abstract class for containers of particles
/** Containers store sets of tuples particles. The degree of the tuple
    (ie whether each tuple containers one, two, three or four
    particles) is constant for each container. That is, a
    SingletonContainer is a set of single particles, a PairContainer
    is a set of pairs of particles etc.

    These sets can come from a variety of sources, such as
    - user-provided lists, eg IMP::core::ListSingletonContainer
    - operations on other containers eg IMP::core::PairContainerSet
    - computations based on particle attributes eg IMP::Core::ClosePairContainer

    Containers provide a variety of methods to
    - get the number of tuples
    - get the ith tuple
    - iterate through the tuples
    - determine if a tuple is contained in the set

    \note Containers store \em sets and so are fundamentally unordered.
 */
class IMPEXPORT Container : public Object
{
public:
  Container(std::string name="Container %1%");

  //! Get the set of containers read by this one
  virtual ContainersTemp get_input_containers() const=0;
  //! Get wether the set of particles changed since last evaluation
  virtual bool get_contained_particles_changed() const=0;
  //! Get contained particles
  /** Get a list of all particles contained in this one,
      given that the input containers are up to date.
  */
  virtual ParticlesTemp get_contained_particles() const=0;

  IMP_REF_COUNTED_DESTRUCTOR(Container);
};

IMP_END_NAMESPACE

#endif  /* IMP_CONTAINER_BASE_H */
