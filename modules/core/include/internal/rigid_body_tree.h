/**
 *  \file rigid_pair_score.h
 *  \brief utilities for rigid pair scores.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODY_TREE_H
#define IMPCORE_INTERNAL_RIGID_BODY_TREE_H

#include "../core_config.h"
#include "../XYZ.h"
#include "../rigid_bodies.h"
#include <IMP/algebra/Sphere3D.h>
#include <IMP/compatibility/set.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

class IMPCOREEXPORT RigidBodyHierarchy: public Object {
  RigidBody rb_;
  struct Data {
    std::vector<int> children_;
    algebra::SphereD<3> s_;
  };
  std::vector<Data> tree_;
  ParticlesTemp constituents_;

  typedef std::vector<unsigned int> SphereIndexes;
  typedef std::vector<SphereIndexes> SpheresSplit;
  SpheresSplit divide_spheres(const std::vector< algebra::SphereD<3> > &ss,
                              const SphereIndexes &s);
  void set_sphere(unsigned int ni, const algebra::SphereD<3> &s);
  void set_leaf(unsigned int ni, const std::vector<unsigned int> &ids);
  unsigned int add_children(unsigned int ni, unsigned int num_children);
  void validate_internal(int cur, algebra::Sphere3Ds bounds) const;
 public:
  algebra::SphereD<3> get_sphere(unsigned int i) const {
    IMP_INTERNAL_CHECK(i < tree_.size(), "Out of spheres vector");
    IMP_CHECK_OBJECT(rb_.get_particle());
    algebra::SphereD<3> ret(rb_.get_reference_frame()
                               .get_global_coordinates(tree_[i]
                                                       .s_.get_center()),
                            tree_[i].s_.get_radius());
    return ret;
  }
  bool get_is_leaf(unsigned int ni) const;
  unsigned int get_number_of_particles(unsigned int ni) const;
  unsigned int get_number_of_children(unsigned int ni) const;
  unsigned int get_child(unsigned int ni, unsigned int i) const;
  Particle* get_particle(unsigned int ni, unsigned int i) const;
  std::vector<algebra::SphereD<3> > get_all_spheres() const;
  RigidBodyHierarchy(RigidBody rb, const ParticlesTemp &constituents);
  std::vector<algebra::SphereD<3> > get_tree() const;
  bool get_constituents_match( ParticlesTemp ps) const {
    if (ps.size() != constituents_.size()) return false;
    std::sort(ps.begin(), ps.end());
    ParticlesTemp un;
    std::set_union(ps.begin(), ps.end(),
                   constituents_.begin(), constituents_.end(),
                   std::back_inserter(un));
    return (un.size()==ps.size());
  }
  const ParticlesTemp &get_constituents() const {
    return constituents_;
  }
  IMP_OBJECT(RigidBodyHierarchy);
  // for testing
  ParticlesTemp get_particles(unsigned int i) const;
  void validate() const;
};

IMPCOREEXPORT Particle* closest_particle(const RigidBodyHierarchy *da,
                                         XYZR pt);


IMPCOREEXPORT ParticlePair closest_pair(const RigidBodyHierarchy *da,
                                        const RigidBodyHierarchy *db);

IMPCOREEXPORT
ParticlePairsTemp close_pairs(const RigidBodyHierarchy *da,
                              const RigidBodyHierarchy *db,
                              double dist);
IMPCOREEXPORT
ParticlesTemp close_particles(const RigidBodyHierarchy *da,
                              XYZR pt, double dist);


IMPCOREEXPORT
RigidBodyHierarchy *get_rigid_body_hierarchy(RigidBody rb,
                                             const ParticlesTemp &constituents,
                                             ObjectKey mykey= ObjectKey());

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_BODY_TREE_H */
