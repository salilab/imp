/**
 *  \file domino/particle_states.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_PARTICLE_STATES_H
#define IMPDOMINO_PARTICLE_STATES_H

#include "domino_config.h"
#include "domino_macros.h"
#include "Subset.h"
#include "SubsetState.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/SingletonContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/internal/OwnerPointer.h>
#include <map>

IMPDOMINO_BEGIN_NAMESPACE
/** Handle the states for a particular particle (or "class" of
    particles. For example a state enumerator class could take
    a bounding box and a number,n, and generate n points in the
    bounding box. Then the get_number function woudld return
    n and update_to_state would modify the particle to have the
    coordiantes for state i.
 */
class IMPDOMINOEXPORT ParticleStates: public Object {
public:
  ParticleStates(std::string name="ParticleStates %1%"): Object(name){}
  virtual unsigned int get_number_of_particle_states() const=0;
  virtual void load_particle_state(unsigned int, Particle*) const=0;
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
class IMPDOMINOEXPORT ParticleStatesTable: public Object {
  typedef std::map<Particle*, IMP::internal::OwnerPointer<ParticleStates> > Map;
  Map enumerators_;
  friend class DominoSampler;
public:
  // implementation methods use this to get the enumerator
  ParticleStates* get_particle_states(Particle *p) const {
    IMP_USAGE_CHECK(enumerators_.find(p) != enumerators_.end(),
                    "I don't know about particle " << p->get_name());
    return enumerators_.find(p)->second;
  }
  bool get_has_particle(Particle *p) const {
    return enumerators_.find(p) != enumerators_.end();
  }
  ParticlesTemp get_particles() const {
    ParticlesTemp ret;
    ret.reserve(enumerators_.size());
    for (Map::const_iterator it= enumerators_.begin();
         it != enumerators_.end(); ++it) {
      ret.push_back(it->first);
    }
    return ret;
  }
  //! Return the subset corresponding to all the particles
  Subset get_subset() const {
    return Subset(get_particles(), true);
  }
  void set_particle_states(Particle *p, ParticleStates *e) {
    IMP_USAGE_CHECK(enumerators_.find(p) == enumerators_.end(),
                    "Enumerator already set for particle " << p->get_name());
    IMP_USAGE_CHECK(e->get_number_of_particle_states() >0,
                    "Cannot have 0 states for a particle: \""
                    << p->get_name() << "\"\n");
    enumerators_[p]=e;
  }
  IMP_OBJECT(ParticleStatesTable);
};

IMP_OBJECTS(ParticleStatesTable, ParticleStatesTables);


/** Store the state index in the particle. The particle must
    already have the attribute in question.
*/
class IMPDOMINOEXPORT IndexStates: public ParticleStates {
  IntKey k_;
  unsigned int n_;
public:
  IndexStates(unsigned int n): ParticleStates("IndexStates %1%"), n_(n){}
  IMP_PARTICLE_STATES(IndexStates);
};

/** Store a set of states which explicitly define the XYZ coordinates of
    the particle in question.
*/
class IMPDOMINOEXPORT XYZStates: public ParticleStates {
  algebra::Vector3Ds states_;
public:
  XYZStates(const algebra::Vector3Ds &states):
    ParticleStates("XYZStates %1%"), states_(states){}
  IMP_PARTICLE_STATES(XYZStates);
};

/** Store a set of states which explicitly define the
    transformation coordinates of the particle in question.
*/
class IMPDOMINOEXPORT RigidBodyStates: public ParticleStates {
  algebra::ReferenceFrame3Ds states_;
public:
  RigidBodyStates(const algebra::ReferenceFrame3Ds &states):
    ParticleStates("RigidBodyStates %1%"), states_(states){}
  IMP_PARTICLE_STATES(RigidBodyStates);
};

/** Combine two particle states together. They must both have the same
    number of states.
*/
class IMPDOMINOEXPORT CompoundStates: public ParticleStates {
  IMP::internal::OwnerPointer<ParticleStates> a_, b_;
public:
  CompoundStates(ParticleStates* a, ParticleStates *b):
    ParticleStates("CompoundStates %1%"), a_(a), b_(b){}
  IMP_PARTICLE_STATES(CompoundStates);
};


/** Load particle states for a set of particles based on the state
    index of a single particle, This can be used to implement compound
    objects (like rigid bodies), where state i of the particle being
    sampled causes a set of representation balls to be moved to
    certain locations.
*/
class IMPDOMINOEXPORT RecursiveStates: public ParticleStates {
  Subset s_;
  SubsetStates ss_;
  IMP::internal::OwnerPointer<ParticleStatesTable> pst_;
  ScopedScoreState sss_;
 public:
  RecursiveStates(Particle *p,
                  Subset s, const SubsetStates &ss,
                  ParticleStatesTable * pst);
  IMP_PARTICLE_STATES(RecursiveStates);
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_PARTICLE_STATES_H */
