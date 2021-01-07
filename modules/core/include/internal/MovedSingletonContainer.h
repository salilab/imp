/**
 *  \file MovedSingletonContainer.h
 *  \brief Keep track of the maximum change of a set of attributes
 *         for e.g., incremental updateing of close pair containers.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_MOVED_SINGLETON_CONTAINER_H
#define IMPCORE_INTERNAL_MOVED_SINGLETON_CONTAINER_H

#include <IMP/core/core_config.h>

#include <IMP/ScoreState.h>
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/internal/ListLikeContainer.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/Pointer.h>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "../XYZR.h"
#include "../rigid_bodies.h"
#include <IMP/internal/container_helpers.h>
#include <vector>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

class IMPCOREEXPORT MovedSingletonContainer
    : public IMP::internal::ListLikeContainer<SingletonContainer> {
 private:
  double threshold_;
  Pointer<SingletonContainer> pc_;
  std::size_t pc_version_;
  PointerMember<ScoreState> score_state_;

  virtual ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;

  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  virtual ParticleIndexes get_range_indexes() const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(MovedSingletonContainer);

  //! return all particles that may have moved by more than get_threshold()
  //! by some metrics
  virtual ParticleIndexes do_get_moved() = 0;

  virtual void do_reset_all() = 0;

  virtual void do_reset_moved() = 0;

  virtual ParticleIndexes do_initialize() = 0;

  virtual ModelObjectsTemp get_extra_inputs() const = 0;

 public:

  ModelObjectsTemp get_score_state_inputs() const;

  void do_score_state_before_evaluate();

  void do_score_state_after_evaluate();

  void initialize();

  virtual void validate() const = 0;

  MovedSingletonContainer(SingletonContainer *pc, double threshold,
                          std::string name);

  //! Measure differences from the current value.
  void reset();

  //! Reset the positions for the moved particles
  void reset_moved();

  //! Return the container storing the particles
  SingletonContainer *get_singleton_container() const { return pc_; }

  //! set threshold beyond which movement is reported
  void set_threshold(double d);

  //! get threshold beyond which movement is reported
  double get_threshold() const { return threshold_; }
#ifndef IMP_DOXYGEN
  Restraints create_decomposition(SingletonScore *) const {
    IMP_NOT_IMPLEMENTED;
  }
#endif
};

//! track the movement of spherical particles whose surface points
//! moved by more than some threshold. If the sphere radius increased
//! or decreased, it is considered as positive or negative surface
//! movement, respectively.
//!
//! Provides a conservative estimate (= might include particles that
//! moved by less than threshold)
class IMPCOREEXPORT XYZRMovedSingletonContainer
    : public MovedSingletonContainer {
  Vector<algebra::Sphere3D> backup_;
  boost::unordered_set<int> moved_;

  //! return any particles whose center
  //! may have moved by more than get_threshold()
  virtual ParticleIndexes do_get_moved();

  virtual void do_reset_all();

  virtual void do_reset_moved();

  virtual ParticleIndexes do_initialize();

  ModelObjectsTemp get_extra_inputs() const {
    return ModelObjectsTemp();
  }

 public:
  virtual void validate() const;

  //! Track the changes of contained particles, such that at least all
  //! particles that moved by threshold are tracked
  XYZRMovedSingletonContainer(SingletonContainer *pc, double threshold);
};

//! track the movements of the members of a set of rigid bodies
//! get_contents() returns any members that may have moved recently by threshold
class IMPCOREEXPORT RigidMovedSingletonContainer
    : public MovedSingletonContainer {

  // last reference frame and radius data for each body in bodies:
  Vector<std::pair<algebra::Sphere3D, algebra::Rotation3D> > backup_;

  // rigid bodies whose members movements are tracked
  ParticleIndexes bodies_;

  // bodies_ indexes of rigid bodies whose members may have moved recently by
  // more than threshold:
  boost::unordered_set<int> moved_;

  // members of each rigid body in bodies_:
  boost::unordered_map<ParticleIndex, ParticleIndexes>
      rbs_members_;

  //! return all rigid body members that may have moved
  //! by more than get_threshold() (since last reset?)
  virtual ParticleIndexes do_get_moved();

  //! update backup_ with the current reference frames of all
  //! bodies_, clear moved_
  virtual void do_reset_all();

  //! update backup_ only for moved_ bodies, clear moved_
  virtual void do_reset_moved();

  virtual ParticleIndexes do_initialize();
  void do_initialize_particle(ParticleIndex pi);
  virtual void validate() const;

  /**
     If internal checks are on, compare the global cartesian coordinates of all members of
     rbs with their global coordinates in the reference frame specified by s, and verify
     that none moved by more than the upper_bound d. Otherwise, throw an exception.

     @param rbs a rigid body
     @param s a tuple of a sphere and rotation, specifying the old reference frame
            (origin at sphere center and rotation s.second)
     @param d estimated upper-bound on movement

     \throw InternalException if any of the members moved by more than d units.
   */
  void check_upper_bound(core::RigidBody rbs,
                      std::pair<algebra::Sphere3D, algebra::Rotation3D> s,
                      double d) const;

  //! return an upper bound on the movement of any particle compared to last iteration
  double get_distance_upper_bound(unsigned int i) const;

  std::pair<algebra::Sphere3D, algebra::Rotation3D> get_data(
      ParticleIndex p) const {
    if (!core::RigidBody::get_is_setup(get_model(), p)) {
      return std::make_pair(core::XYZR(get_model(), p).get_sphere(),
                            algebra::Rotation3D());
    } else {
      return std::make_pair(core::XYZR(get_model(), p).get_sphere(),
                            core::RigidBody(get_model(), p)
                                .get_reference_frame()
                                .get_transformation_to()
                                .get_rotation());
    }
  }
  ModelObjectsTemp get_extra_inputs() const;

 public:
  //! Track the changes with the specified keys.
  RigidMovedSingletonContainer(SingletonContainer *pc, double threshold);
};
IMP_OBJECTS(MovedSingletonContainer, MovedSingletonContainers);

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_MOVED_SINGLETON_CONTAINER_H */
