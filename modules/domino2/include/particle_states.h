/**
 *  \file domino2/particle_states.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_PARTICLE_STATES_H
#define IMPDOMINO2_PARTICLE_STATES_H

#include "domino2_config.h"
#include "domino2_macros.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/internal/OwnerPointer.h>
#include <map>

IMPDOMINO2_BEGIN_NAMESPACE
/** The set of particles defining a node in the junction tree. A
    SingletonContainer is used so that the pointer value uniquely
    identifies a node (and the container has a name so that
    it can be nicely written for display).
 */
typedef SingletonContainer Subset;




/** Handle the states for a particular particle (or "class" of
    particles. For example a state enumerator class could take
    a bounding box and a number,n, and generate n points in the
    bounding box. Then the get_number function woudld return
    n and update_to_state would modify the particle to have the
    coordiantes for state i.
 */
class IMPDOMINO2EXPORT ParticleStates: public Object {
public:
  virtual unsigned int get_number_of_states() const=0;
  virtual void load_state(unsigned int, Particle*) const=0;
  virtual ~ParticleStates();
};

IMP_OBJECTS(ParticleStates, ParticleStatesList);

/** Store the association between particles and the classes
    which manage their states. I'm not a huge fan of having
    this class, but I haven't thought of a better way to store
    the information that is easily exposed to python
    and gets to all the right places. It is initialized internally
    in the DominoSampler
 */
class IMPDOMINO2EXPORT ParticleStatesTable: public Object {
  typedef std::map<Particle*, IMP::internal::OwnerPointer<ParticleStates> > Map;
  Map enumerators_;
  friend class DominoSampler;
  void set_enumerator(Particle *p, ParticleStates *e) {
    IMP_USAGE_CHECK(enumerators_.find(p) == enumerators_.end(),
                    "Enumerator already set for particle " << p->get_name());
    enumerators_[p]=e;
  }
  // This defines the ordering of particles used
  ParticlesTemp get_particles() const {
    ParticlesTemp ret;
    ret.reserve(enumerators_.size());
    for (Map::const_iterator it= enumerators_.begin();
         it != enumerators_.end(); ++it) {
      ret.push_back(it->first);
    }
    return ret;
  }
public:
  // implementation methods use this to get the enumerator
  ParticleStates* get_particle_states(Particle *p) const {
    IMP_USAGE_CHECK(enumerators_.find(p) != enumerators_.end(),
                    "I don't know about particle " << p->get_name());
    return enumerators_.find(p)->second;
  }

  IMP_OBJECT(ParticleStatesTable);
};

IMP_OBJECTS(ParticleStatesTable, ParticleStatesTables);

/** Store a set of states which explicitly define the XYZ coordinates of
    the particle in question.
*/
class IMPDOMINO2EXPORT XYZsStates: public ParticleStates {
  algebra::Vector3Ds states_;
public:
  XYZsStates(const algebra::Vector3Ds &states): states_(states){}
  IMP_PARTICLE_STATES(XYZsStates);
};
IMP_OBJECTS(XYZsStates, XYZsStatesList);


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_PARTICLE_STATES_H */
