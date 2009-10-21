/**
 *  \file rigid_pair_score.h
 *  \brief utilities for rigid pair scores.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODY_TREE_H
#define IMPCORE_INTERNAL_RIGID_BODY_TREE_H

#include "../config.h"
#include "../XYZ.h"
#include "../Hierarchy.h"
#include "../rigid_bodies.h"
#include <IMP/algebra/Sphere3D.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

class IMPCOREEXPORT RigidBodyHierarchy: public Object {
  RigidBody rb_;
  IMP::internal::OwnerPointer<Refiner> r_;
  struct Data {
    std::vector<int> children_;
    algebra::Sphere3D s_;
  };
  std::vector<Data> tree_;

  typedef std::vector<unsigned int> SphereIndexes;
  typedef std::vector<SphereIndexes> SpheresSplit;
  SpheresSplit divide_spheres(const algebra::Sphere3Ds &ss,
                              const SphereIndexes &s);
  void set_sphere(unsigned int ni, const algebra::Sphere3D &s);
  void set_leaf(unsigned int ni, const std::vector<unsigned int> &ids);
  unsigned int add_children(unsigned int ni, unsigned int num_children);
 public:
  algebra::Sphere3D get_sphere(unsigned int i) const {
    IMP_INTERNAL_CHECK(i < tree_.size(), "Out of spheres vector");
    return algebra::Sphere3D(rb_.get_transformation()
                             .transform(tree_[i].s_.get_center()),
                             tree_[i].s_.get_radius());
  }
  bool get_is_leaf(unsigned int ni) const;
  unsigned int get_number_of_particles(unsigned int ni) const;
  unsigned int get_number_of_children(unsigned int ni) const;
  unsigned int get_child(unsigned int ni, unsigned int i) const;
  Particle* get_particle(unsigned int ni, unsigned int i) const;
  std::vector<algebra::Sphere3D> get_all_spheres() const;
  RigidBodyHierarchy(RigidBody rb, Refiner *r);
  algebra::Sphere3Ds get_tree() const;
  IMP_OBJECT(RigidBodyHierarchy, get_module_version_info());
  // for testing
  ParticlesTemp get_particles(unsigned int i) const;
};

namespace {
  template <class F>
  void
  process_one(const RigidBodyHierarchy *da,
              const RigidBodyHierarchy *db,
              const F &f,
              unsigned int ci,
              unsigned int cj,
              std::vector<std::pair<int, int> > &stack,
              double distance) {
    if (da->get_is_leaf(ci) && db->get_is_leaf(cj)) {
      for (unsigned int k=0; k< da->get_number_of_particles(ci); ++k) {
        Particle *pk=da->get_particle(ci, k);
        algebra::Sphere3D sk(XYZ(pk).get_coordinates(),
                             XYZR(pk).get_radius()+distance*.5);
        for (unsigned int l=0; l< db->get_number_of_particles(cj); ++l) {
          Particle *pl=db->get_particle(cj, l);
          algebra::Sphere3D sl(XYZ(pl).get_coordinates(),
                               XYZR(pl).get_radius()+distance*.5);
          /*IMP_LOG(VERBOSE, "Trying member particles " << pk->get_name()
            << " and " << pl->get_name() << std::endl);*/
          if (interiors_intersect(sk, sl)) {
            f(pk, pl);
          } else {
            /*IMP_LOG(VERBOSE, "Spheres do not interesct " << sk << " | " << sl
              << std::endl);*/
          }
        }
      }
    } else {
      stack.push_back(std::make_pair(ci, cj));
    }
  }


  template <class F, bool SWAP>
  void
  process_one(const RigidBodyHierarchy *da,
              XYZR db,
              const F &f,
              unsigned int ci,
              std::vector<int > &stack,
              double distance)
  {
    if (da->get_is_leaf(ci)) {
      for (unsigned int k=0; k< da->get_number_of_particles(ci); ++k) {
        Particle *pk=da->get_particle(ci, k);
        algebra::Sphere3D sk(XYZ(pk).get_coordinates(),
                             XYZR(pk).get_radius()+distance*.5);
        algebra::Sphere3D sl(db.get_coordinates(),
                             db.get_radius()+distance*.5);
        /*IMP_LOG(VERBOSE, "Trying member particles " << pk->get_name()
          << " and " << pl->get_name() << std::endl);*/
        if (interiors_intersect(sk, sl)) {
          if (SWAP) {
            f(db, pk);
          } else {
            f(pk, db);
          }
        } else {
          /*IMP_LOG(VERBOSE, "Spheres do not interesct " << sk << " | " << sl
            << std::endl);*/
        }
      }
    } else {
      stack.push_back(ci);
    }
  }
}


template <class F>
void apply_to_nearby(const RigidBodyHierarchy *da,
                     const RigidBodyHierarchy *db,
                     double distance, const F&f) {
  std::vector<std::pair<int, int> > stack;
  process_one(da,db,f, 0,0,stack, distance);
  while (!stack.empty()) {
    std::pair<int, int> cur= stack.back();
    stack.pop_back();
    /*IMP_LOG(VERBOSE, "Processing pair " << cur.first << " "
      << cur.second << std::endl);*/
    for (unsigned int i=0; i< da->get_number_of_children(cur.first);
         ++i) {
      int ci=da->get_child(cur.first, i);
      algebra::Sphere3D si(da->get_sphere(ci).get_center(),
                           da->get_sphere(ci).get_radius()+distance);
      for (unsigned int j=0;
           j< db->get_number_of_children(cur.second); ++j) {
        int cj=db->get_child(cur.second, j);
        algebra::Sphere3D sj = db->get_sphere(cj);
        if (interiors_intersect(si, sj)) {
          process_one(da, db, f, ci, cj, stack, distance);
        } else {
          /*IMP_LOG(VERBOSE, "Rejected " << ci << " " << cj << ": "
            << si << " | " << sj << std::endl);*/
        }
      }
    }
  }
}


template <class F, bool SWAP>
void apply_to_nearby(const RigidBodyHierarchy *da,
                     XYZR db,
                     double distance, const F&f) {
  std::vector<int> stack;
  process_one<F, SWAP>(da,db,f, 0,stack, distance);
  while (!stack.empty()) {
    int cur= stack.back();
    stack.pop_back();
    /*IMP_LOG(VERBOSE, "Processing pair " << cur.first << " "
      << cur.second << std::endl);*/
    for (unsigned int i=0; i< da->get_number_of_children(cur);
         ++i) {
      int ci=da->get_child(cur, i);
      algebra::Sphere3D si(da->get_sphere(ci).get_center(),
                           da->get_sphere(ci).get_radius()
                           + distance); // check which tree
      algebra::Sphere3D sj = db.get_sphere();
      if (interiors_intersect(si, sj)) {
        process_one<F, SWAP>(da, db, f, ci, stack, distance);
      } else {
        /*IMP_LOG(VERBOSE, "Rejected " << ci << " " << cj << ": "
          << si << " | " << sj << std::endl);*/
      }
    }
  }
}

IMPCOREEXPORT Particle* closest_particle(const RigidBodyHierarchy *da,
                                         XYZR pt);


IMPCOREEXPORT ParticlePair closest_pair(const RigidBodyHierarchy *da,
                                        const RigidBodyHierarchy *db);


IMPCOREEXPORT ObjectKey get_rigid_body_hierarchy_key(Refiner *r);

inline
RigidBodyHierarchy *get_rigid_body_hierarchy(RigidBody rb,
                                             Refiner*r, ObjectKey k) {
  if (rb.get_particle()->has_attribute(k)) {
    return object_cast<RigidBodyHierarchy>(rb.get_particle()->get_value(k));
  } else {
    RigidBodyHierarchy *h= new RigidBodyHierarchy(rb, r);
    rb.get_particle()->add_cache_attribute(k, h);
    return h;
  }
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_BODY_TREE_H */
