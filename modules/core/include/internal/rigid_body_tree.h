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
#include <queue>

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


struct LessFirst {
  template <class A>
  bool operator()(const A &a, const A &b) const{
    return a.first < b.first;
  }
};
inline double
distance_bound(const RigidBodyHierarchy *da, unsigned int i,
               XYZR b) {
  algebra::SphereD<3> s= da->get_sphere(i);
  double rd= algebra::get_distance(s, b.get_sphere());
  return rd;
}

inline double
distance_bound(const RigidBodyHierarchy *da, unsigned int i,
               const RigidBodyHierarchy *db, unsigned int j) {
  algebra::SphereD<3> sa= da->get_sphere(i);
  algebra::SphereD<3> sb= db->get_sphere(j);
  double rd= algebra::get_distance(sa, sb);
  return rd;
}

template <class Sink>
inline void fill_close_pairs(const RigidBodyHierarchy *da,
                              const RigidBodyHierarchy *db,
                              double dist,
                              Sink sink) {
  typedef std::pair<int,int> IP;
  typedef std::pair<double, IP> QP;
  std::priority_queue<QP, std::vector<QP>, LessFirst> queue;
  double d= distance_bound(da, 0, db, 0);
  queue.push(QP(d, IP(0,0)));
  ParticlePairsTemp ret;
  do {
    QP v= queue.top();
    queue.pop();
    if (v.first > dist) break;
    /*IMP_LOG(TERSE, "Trying pair " << v.second.first << " " << v.second.second
      << std::endl);*/
    if (da->get_is_leaf(v.second.first) && db->get_is_leaf(v.second.second)) {
      for (unsigned int i=0;
           i< da->get_number_of_particles(v.second.first); ++i) {
        XYZR deca(da->get_particle(v.second.first, i));
        for (unsigned int j=0;
             j< db->get_number_of_particles(v.second.second); ++j) {
          XYZR decb(db->get_particle(v.second.second, j));
          double d= get_distance(deca, decb);
          if (d < dist) {
            if (!sink(ParticlePair(deca, decb))) {
              return;
            }
            /*std::cout << "Updating threshold to " << best_d
              << " due to pair " << bp << std::endl;*/
          }
        }
      }
    } else if (da->get_is_leaf(v.second.first)) {
        for (unsigned int j=0;
             j< db->get_number_of_children(v.second.second); ++j) {
          unsigned int child = db->get_child(v.second.second, j);
          double d= distance_bound(da, v.second.first,
                                                     db, child);
          if (d < dist) {
            queue.push(QP(d, IP(v.second.first, child)));
          }
        }
    } else if (db->get_is_leaf(v.second.second)) {
      for (unsigned int i=0;
           i< da->get_number_of_children(v.second.first); ++i) {
        unsigned int child = da->get_child(v.second.first, i);
          double d= distance_bound(da, child,
                                   db, v.second.second);
          if (d < dist) {
            queue.push(QP(d, IP(child, v.second.second)));
          }
        }
    } else {
      for (unsigned int i=0;
           i< da->get_number_of_children(v.second.first); ++i) {
        unsigned int childa = da->get_child(v.second.first, i);
        for (unsigned int j=0;
             j< db->get_number_of_children(v.second.second); ++j) {
          unsigned int childb = db->get_child(v.second.second, j);
          double d= distance_bound(da, childa,
                                   db, childb);
          if (d < dist) {
            queue.push(QP(d, IP(childa, childb)));
          }
        }
      }
    }
  } while (!queue.empty());
}


IMPCOREEXPORT
ParticlePairsTemp close_pairs(const RigidBodyHierarchy *da,
                              const RigidBodyHierarchy *db,
                              double dist);


template <class Sink>
inline void fill_close_particles(const RigidBodyHierarchy *da,
                              XYZR pt, double dist,
                              Sink sink) {
  typedef std::pair<double, int> QP;
  std::priority_queue<QP, std::vector<QP>, LessFirst> queue;
  double d= distance_bound(da, 0, pt);
  queue.push(QP(d, 0));
  do {
    std::pair<double, int> v= queue.top();
    queue.pop();
    if (v.first > dist) break;
    if (da->get_is_leaf(v.second)) {
      for (unsigned int i=0; i< da->get_number_of_particles(v.second);
           ++i) {
        Particle *p= da->get_particle(v.second, i);
        XYZR dd(p);
        double d= get_distance(dd, pt);
        if (d < dist) {
          if (!sink(p)) {
            return;
          }
        }
      }
    } else {
      for (unsigned int i=0; i< da->get_number_of_children(v.second);
           ++i) {
        unsigned int c= da->get_child(v.second, i);
        double d= distance_bound(da, c, pt);
        if (d < dist) {
          queue.push(QP(d, c));
        }
      }
    }
  } while (!queue.empty());
}

IMPCOREEXPORT
ParticlesTemp close_particles(const RigidBodyHierarchy *da,
                              XYZR pt, double dist);

IMPCOREEXPORT
RigidBodyHierarchy *get_rigid_body_hierarchy(RigidBody rb,
                                             const ParticlesTemp &constituents,
                                             ObjectKey mykey= ObjectKey());

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_BODY_TREE_H */
