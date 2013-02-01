/**
 *  \file IMP/domino/particle_states.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_PARTICLE_STATES_H
#define IMPDOMINO_PARTICLE_STATES_H

#include <IMP/domino/domino_config.h>
#include "domino_macros.h"
#include "Subset.h"
#include "Assignment.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/SingletonContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/internal/OwnerPointer.h>
#include <IMP/base/Vector.h>
#include <IMP/base/map.h>
#include <IMP/base/InputAdaptor.h>
#include <IMP/algebra/vector_search.h>

IMPDOMINO_BEGIN_NAMESPACE
/** Handle the states for a particular particle (or "class" of
    particles. For example a state enumerator class could take
    a bounding box and a number,n, and generate n points in the
    bounding box. Then the get_number function woudld return
    n and update_to_state would modify the particle to have the
    coordiantes for state i.
 */
class IMPDOMINOEXPORT ParticleStates: public IMP::base::Object {
public:
  ParticleStates(std::string name="ParticleStates %1%"): Object(name){}
  virtual unsigned int get_number_of_particle_states() const=0;
  virtual void load_particle_state(unsigned int, Particle*) const=0;
  //! Return an embedding of the state
  /** By default this just returns a 1D vector containing the index.
      The vector needs to have the same dimension for each value of
      i.
   */
  virtual algebra::VectorKD get_embedding(unsigned int i) const {
    Floats f(1,i);
    return algebra::VectorKD(f.begin(), f.end());
  }
  //! Return the state closest to a given embedding
  virtual unsigned int get_nearest_state(const algebra::VectorKD &v) const {
    IMP_INTERNAL_CHECK(v.get_dimension()==1, "This is not a defaultly produced"
                       << " embedding.");
    IMP_INTERNAL_CHECK(v[0] >=0 && v[0] < get_number_of_particle_states(),
                       "Out of range state found, this is not a default "
                       << "embedding.");
    return static_cast<unsigned int>(v[0]);
  }
  virtual ~ParticleStates();
};

IMP_OBJECTS(ParticleStates, ParticleStatesList);

/** Store the association between particles and the classes
    which manage their states. I'm not a huge fan of having
    this class, but I haven't thought of a better way to store
    the information that is easily exposed to python
    and gets to all the right places. It is initialized internally
    in the DominoSampler.
 */
class IMPDOMINOEXPORT ParticleStatesTable: public IMP::base::Object {
  typedef IMP::base::map<Particle*,
                   IMP::OwnerPointer<ParticleStates> > Map;
  Map enumerators_;
  friend class DominoSampler;
 public:
  ParticleStatesTable(): Object("ParticleStatesTable%1%"){}
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
    std::sort(ret.begin(), ret.end());
    return ret;
  }
  //! Return the subset corresponding to all the particles
  Subset get_subset() const {
    return Subset(get_particles());
  }
  /** One can set the states more than once. If you do that, be
      careful.
  */
  void set_particle_states(Particle *p, ParticleStates *e) {
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
  unsigned int n_;
  IntKey k_;
public:
  /** n is the number of states and k is the attribute key
      to use.*/
  IndexStates(unsigned int n, IntKey k=IntKey("state")):
    ParticleStates("IndexStates %1%"), n_(n), k_(k){}
  IMP_PARTICLE_STATES(IndexStates);
};

/** Store a set of states which explicitly define the XYZ coordinates of
    the particle in question.
*/
class IMPDOMINOEXPORT XYZStates: public ParticleStates {
  algebra::Vector3Ds states_;
  base::OwnerPointer<algebra::NearestNeighbor3D> nn_;
public:
  XYZStates(const algebra::Vector3Ds &states):
      ParticleStates("XYZStates %1%"), states_(states),
      nn_(new algebra::NearestNeighbor3D(states)) {}
  algebra::Vector3D get_vector(unsigned int i) const {
    IMP_USAGE_CHECK(i < states_.size(),
                    "Out of range");
    return states_[i];
  }
  algebra::VectorKD get_embedding(unsigned int i) const {
    IMP_USAGE_CHECK(i < states_.size(),
                    "Out of range");
    return states_[i];
  }
  unsigned int get_nearest_state(const algebra::VectorKD &v) const {
    return nn_->get_nearest_neighbors(v, 1)[0];
  }
  IMP_PARTICLE_STATES(XYZStates);
};

/** Store a set of states which explicitly define the
    transformation coordinates of the particle in question.
*/
class IMPDOMINOEXPORT RigidBodyStates: public ParticleStates {
  algebra::ReferenceFrame3Ds states_;
  double scale_;
  base::OwnerPointer<algebra::NearestNeighbor6D> nn_;
public:
  RigidBodyStates(const algebra::ReferenceFrame3Ds &states,
                  double scale=1);
  algebra::ReferenceFrame3D get_reference_frame(unsigned int i) const {
    IMP_USAGE_CHECK(i < states_.size(),
                    "Out of range");
    return states_[i];
  }
  algebra::VectorKD get_embedding(unsigned int i) const;
  unsigned int get_nearest_state(const algebra::VectorKD &v) const;
  IMP_PARTICLE_STATES(RigidBodyStates);
};

/** Store a set of states which explicitly define the
    internal transformation of a rigid member
*/
class IMPDOMINOEXPORT NestedRigidBodyStates: public ParticleStates {
  algebra::Transformation3Ds states_;//states of a nested rigid body
  double scale_;
  base::OwnerPointer<algebra::NearestNeighbor6D> nn_;
public:
  /**
     \param[in] states states of a rigid member with respect to its parent
     \param[in] scale an estimate of the radius of the rigid bodies used.
     Unfortunately, this is hard to determine automatically in the right
     place.
   */
  NestedRigidBodyStates(const algebra::Transformation3Ds &states,
                        double scale=100);
  algebra::Transformation3D get_transformation(unsigned int i) const {
    IMP_USAGE_CHECK(i < states_.size(),
                    "Out of range");
    return states_[i];
  }
  algebra::VectorKD get_embedding(unsigned int i) const;
  unsigned int get_nearest_state(const algebra::VectorKD &v) const;
  IMP_PARTICLE_STATES(NestedRigidBodyStates);
};

/** Combine two particle states together. They must both have the same
    number of states.
*/
class IMPDOMINOEXPORT CompoundStates: public ParticleStates {
  IMP::OwnerPointer<ParticleStates> a_, b_;
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
  Assignments ss_;
  IMP::OwnerPointer<ParticleStatesTable> pst_;
  ScopedScoreState sss_;
 public:
  RecursiveStates(Particle *p,
                  Subset s, const Assignments &ss,
                  ParticleStatesTable * pst);
  IMP_PARTICLE_STATES(RecursiveStates);
};


/** Permute the states of a particle. This might be useful when
    trying to sample from a too large set of Assignments. However,
    it will break many filters, so use with care.
*/
class IMPDOMINOEXPORT PermutationStates: public ParticleStates {
  IMP::OwnerPointer<ParticleStates> inner_;
  IMP::base::Vector<int> permutation_;
 public:
  PermutationStates(ParticleStates *inner);
  /** Return the index of the ith state in the inner ParticleState
      object.*/
  unsigned int get_inner_state(unsigned int i) const {
    IMP_CHECK_OBJECT(this);
    IMP_USAGE_CHECK(i < permutation_.size(), "Out of range inner state");
    unsigned int cur= permutation_[i];
    IMP_INTERNAL_CHECK(cur < inner_->get_number_of_particle_states(),
                       "Out of range state returned. This is perplexing.");
    return cur;
  }
  IMP_PARTICLE_STATES(PermutationStates);
};

#ifndef IMP_DOXYGEN
inline unsigned int
PermutationStates::get_number_of_particle_states() const {
  return inner_->get_number_of_particle_states();
}
inline void
PermutationStates::load_particle_state(unsigned int i, Particle *p) const {
  return inner_->load_particle_state(get_inner_state(i), p);
}
#endif


/** Accept either particles, decorators or ParticleStatesTable
    as an imput to define a list of particle.*/
class ParticlesAdaptor:
#ifndef SWIG
  public ParticlesTemp, public base::InputAdaptor
#else
  public base::InputAdaptor
#endif
{
public:
  ParticlesAdaptor(ParticleStatesTable *pst):
      ParticlesTemp(pst->get_particles()) {
  }
  ParticlesAdaptor(const ParticlesTemp &ps): ParticlesTemp(ps.begin(),
                                                           ps.end()){}
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_PARTICLE_STATES_H */
