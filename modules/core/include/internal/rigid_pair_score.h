/**
 *  \file rigid_pair_score.h
 *  \brief utilities for rigid pair scores.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_PAIR_SCORE_H
#define IMPCORE_INTERNAL_RIGID_PAIR_SCORE_H

#include "../config.h"
#include "../XYZDecorator.h"
#include "../HierarchyDecorator.h"
#include <IMP/algebra/Sphere3D.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

class IMPCOREEXPORT RigidBodyParticleData {
  struct Data {
    std::vector<int> storage_;
    algebra::Sphere3D s_;
  };
  std::vector<Data> data_;
public:
  RigidBodyParticleData();
  void set_sphere(unsigned int ni, const algebra::Sphere3D &s);
  void set_leaf(unsigned int ni, const std::vector<unsigned int> &ids);
  unsigned int add_children(unsigned int ni, unsigned int num_children);
  bool get_is_leaf(unsigned int ni) const;
  unsigned int get_number_of_particles(unsigned int ni) const;
  unsigned int get_number_of_children(unsigned int ni) const;
  unsigned int get_child(unsigned int ni, unsigned int i) const;
  Particle* get_particle(unsigned int ni, unsigned int i,
                            Particle *p) const;
  const algebra::Sphere3D &get_sphere(unsigned int ni) const;
  void show_tree(std::ostream &out, Particle *p) const;
  std::vector<algebra::Sphere3D> get_spheres() const;
};


typedef std::vector<unsigned int> SphereIndexes;
typedef std::vector<SphereIndexes> SpheresSplit;


class IMPCOREEXPORT RigidBodyCollisionData {
  std::map<Particle*, RigidBodyParticleData> data_;
public:
  // for a non rigid body
  void add_data(Particle *p, const algebra::Sphere3D &s);
  void add_data(Particle *p);
  RigidBodyParticleData &get_data(Particle *p);
  const RigidBodyParticleData &get_data(Particle *p) const;
  bool has_data(Particle *p) const;
  void show(std::ostream &out) const;
  void clear() {data_.clear();}
};


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_PAIR_SCORE_H */
