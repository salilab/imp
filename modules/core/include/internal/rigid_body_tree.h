/**
 *  \file rigid_pair_score.h
 *  \brief utilities for rigid pair scores.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODY_TREE_H
#define IMPCORE_INTERNAL_RIGID_BODY_TREE_H

#include "../core_config.h"
#include "../XYZ.h"
#include "../rigid_bodies.h"
#include <IMP/algebra/Sphere3D.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

class IMPCOREEXPORT RigidBodyHierarchy: public Object {
  RigidBody rb_;
  struct Data {
    std::vector<int> children_;
    algebra::SphereD<3> s_;
  };
  std::vector<Data> tree_;

  typedef std::vector<unsigned int> SphereIndexes;
  typedef std::vector<SphereIndexes> SpheresSplit;
  SpheresSplit divide_spheres(const std::vector< algebra::SphereD<3> > &ss,
                              const SphereIndexes &s);
  void set_sphere(unsigned int ni, const algebra::SphereD<3> &s);
  void set_leaf(unsigned int ni, const std::vector<unsigned int> &ids);
  unsigned int add_children(unsigned int ni, unsigned int num_children);
 public:
  algebra::SphereD<3> get_sphere(unsigned int i) const {
    IMP_INTERNAL_CHECK(i < tree_.size(), "Out of spheres vector");
    return algebra::SphereD<3>(rb_.get_reference_frame()
                               .get_global_coordinates(tree_[i]
                                                       .s_.get_center()),
                             tree_[i].s_.get_radius());
  }
  bool get_is_leaf(unsigned int ni) const;
  unsigned int get_number_of_particles(unsigned int ni) const;
  unsigned int get_number_of_children(unsigned int ni) const;
  unsigned int get_child(unsigned int ni, unsigned int i) const;
  Particle* get_particle(unsigned int ni, unsigned int i) const;
  std::vector<algebra::SphereD<3> > get_all_spheres() const;
  RigidBodyHierarchy(RigidBody rb);
  std::vector<algebra::SphereD<3> > get_tree() const;
  IMP_OBJECT(RigidBodyHierarchy);
  // for testing
  ParticlesTemp get_particles(unsigned int i) const;
};

IMPCOREEXPORT Particle* closest_particle(const RigidBodyHierarchy *da,
                              const IMP::internal::Set<Particle*> &psa,
                                         XYZR pt);


IMPCOREEXPORT ParticlePair closest_pair(const RigidBodyHierarchy *da,
                            const IMP::internal::Set<Particle*> &psa,
                                        const RigidBodyHierarchy *db,
                            const IMP::internal::Set<Particle*> &psb);

IMPCOREEXPORT
ParticlePairsTemp close_pairs(const RigidBodyHierarchy *da,
                              const IMP::internal::Set<Particle*> &psa,
                              const RigidBodyHierarchy *db,
                              const IMP::internal::Set<Particle*> &psb,
                              double dist);
IMPCOREEXPORT
ParticlesTemp close_particles(const RigidBodyHierarchy *da,
                              const IMP::internal::Set<Particle*> &psa,
                              XYZR pt, double dist);

IMPCOREEXPORT ObjectKey get_rigid_body_hierarchy_key();

inline
RigidBodyHierarchy *get_rigid_body_hierarchy(RigidBody rb,
                                             ObjectKey k) {
  if (rb.get_particle()->has_attribute(k)) {
    return object_cast<RigidBodyHierarchy>(rb.get_particle()->get_value(k));
  } else {
    RigidBodyHierarchy *h= new RigidBodyHierarchy(rb);
    rb.get_particle()->add_cache_attribute(k, h);
    return h;
  }
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_BODY_TREE_H */
