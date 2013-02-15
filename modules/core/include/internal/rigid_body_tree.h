/**
 *  \file rigid_pair_score.h
 *  \brief utilities for rigid pair scores.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODY_TREE_H
#define IMPCORE_INTERNAL_RIGID_BODY_TREE_H

#include <IMP/core/core_config.h>
#include "../XYZ.h"
#include "../rigid_bodies.h"
#include <IMP/algebra/Sphere3D.h>
#include <IMP/base/set.h>
#include <queue>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

class IMPCOREEXPORT RigidBodyHierarchy: public IMP::base::Object {
  RigidBody rb_;
  struct Data {
    Ints children_;
    algebra::Sphere3D s_;
  };
  base::Vector<Data> tree_;
  ParticleIndexes constituents_;

  typedef base::Vector<unsigned int> SphereIndexes;
  typedef base::Vector<SphereIndexes> SpheresSplit;
  SpheresSplit divide_spheres(const algebra::Sphere3Ds &ss,
                              const SphereIndexes &s);
  void set_sphere(unsigned int ni, const algebra::Sphere3D &s);
  void set_leaf(unsigned int ni, const ParticleIndexes &ids);
  unsigned int add_children(unsigned int ni, unsigned int num_children);
  ParticleIndexes validate_internal(Model *m, int cur,
                                    algebra::Sphere3Ds bounds) const;
  typedef std::pair<unsigned int, SphereIndexes> Node;
  void build_tree(Model *m, const Node &n,
                  const algebra::Sphere3Ds &spheres,
                  base::Vector<Node> &stack);
 public:
  algebra::Sphere3D get_sphere(unsigned int i) const {
    IMP_INTERNAL_CHECK(i < tree_.size(), "Out of spheres vector");
    IMP_CHECK_OBJECT(rb_.get_particle());
    algebra::Sphere3D ret(rb_.get_reference_frame()
                               .get_global_coordinates(tree_[i]
                                                       .s_.get_center()),
                            tree_[i].s_.get_radius());
    return ret;
  }
  bool get_is_leaf(unsigned int ni) const {
    IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
    IMP_INTERNAL_CHECK(!tree_[ni].children_.empty(),
                       "Everything must have particles or children");
    return tree_[ni].children_[0] < 0;
  }
  unsigned int get_number_of_particles(unsigned int ni) const {
    IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
    IMP_INTERNAL_CHECK(get_is_leaf(ni), "Only leaves have particles");
    return tree_[ni].children_.size();
  }
  unsigned int get_number_of_children(unsigned int ni) const {
    IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
    if (!get_is_leaf(ni)) {
      return tree_[ni].children_.size();
    } else {
      return 1;
    }
  }
  unsigned int get_child(unsigned int ni, unsigned int i) const  {
    IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
    IMP_INTERNAL_CHECK(tree_[ni].children_.size() > i,
                       "Out of range in particle");
    if (!get_is_leaf(ni)) {
      return tree_[ni].children_[i];
    } else {
      return ni;
    }
  }
  ParticleIndex get_particle(unsigned int ni, unsigned int i) const {
    IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
    IMP_INTERNAL_CHECK(tree_[ni].children_.size() > i,
                       "Out of range in particle");
    IMP_INTERNAL_CHECK(tree_[ni].children_[i] < 0,
                       "Not a leaf node");
    ParticleIndex index(std::abs(tree_[ni].children_[i])-1);
    return index;
  }
  algebra::Sphere3Ds get_all_spheres() const;
  RigidBodyHierarchy(RigidBody rb, const ParticleIndexes &constituents);
  algebra::Sphere3Ds get_tree() const;
  bool get_constituents_match(const ParticleIndexes& ps) const {
    if (ps.size() != constituents_.size()) return false;
    ParticleIndexes un;
    std::set_union(ps.begin(), ps.end(),
                   constituents_.begin(), constituents_.end(),
                   std::back_inserter(un));
    return (un.size()==ps.size());
  }
  const ParticleIndexes &get_constituents() const {
    return constituents_;
  }
  IMP_OBJECT(RigidBodyHierarchy);
  // for testing
  ParticleIndexes get_particles(unsigned int i) const {
    ParticleIndexes ret;
    Ints stack(1, i);
    do {
      unsigned int i= stack.back();
      stack.pop_back();
      if (get_is_leaf(i)) {
        for (unsigned int j=0; j< get_number_of_particles(i); ++j) {
          ret.push_back(get_particle(i,j));
        }
      } else {
        for (unsigned int j=0; j< get_number_of_children(i); ++j){
          stack.push_back(get_child(i,j));
        }
      }
    } while (!stack.empty());
    return ret;
  }
  void validate(Model *m) const;
  RigidBody get_rigid_body() const {return rb_;}
  Model *get_model() const {return rb_->get_model();}
};

IMPCOREEXPORT Particle* closest_particle(Model *m,
                                         const RigidBodyHierarchy *da,
                                         XYZR pt);

IMPCOREEXPORT Particle* closest_particle(Model *m, const RigidBodyHierarchy *da,
                                         XYZR pt, double dist
                                         =std::numeric_limits<double>::max());

IMPCOREEXPORT ParticlePair closest_pair(Model *m, const RigidBodyHierarchy *da,
                                        const RigidBodyHierarchy *db);
IMPCOREEXPORT ParticlePair closest_pair(Model *m, const RigidBodyHierarchy *da,
                                        const RigidBodyHierarchy *db,
                              double dist=std::numeric_limits<double>::max());


struct LessFirst {
  template <class A>
  bool operator()(const A &a, const A &b) const{
    return a.first < b.first;
  }
};
inline double
distance_bound(Model *m, const RigidBodyHierarchy *da, unsigned int i,
               ParticleIndex b) {
  algebra::Sphere3D s= da->get_sphere(i);
  double rd= algebra::get_distance(s, m->get_sphere(b));
  return rd;
}

inline double
distance_bound(Model *, const RigidBodyHierarchy *da, unsigned int i,
               const RigidBodyHierarchy *db, unsigned int j) {
  algebra::Sphere3D sa= da->get_sphere(i);
  algebra::Sphere3D sb= db->get_sphere(j);
  double rd= algebra::get_distance(sa, sb);
  return rd;
}

template <class Sink>
inline void fill_close_pairs(Model *m,
                             const RigidBodyHierarchy *da,
                             const RigidBodyHierarchy *db,
                             double dist,
                             Sink sink) {
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    da->validate(m);
    db->validate(m);
  }
  typedef std::pair<int,int> IP;
  typedef std::pair<double, IP> QP;
  std::priority_queue<QP, base::Vector<QP>, LessFirst> queue;
  double d= distance_bound(m, da, 0, db, 0);
  if (d < dist) {
    queue.push(QP(d, IP(0,0)));
  }
  while (!queue.empty()) {
    QP v= queue.top();
    queue.pop();
    /*IMP_LOG_TERSE( "Trying pair " << v.second.first << " " << v.second.second
      << std::endl);*/
    if (da->get_is_leaf(v.second.first) && db->get_is_leaf(v.second.second)) {
      for (unsigned int i=0;
           i< da->get_number_of_particles(v.second.first); ++i) {
        ParticleIndex deca(da->get_particle(v.second.first, i));
        for (unsigned int j=0;
             j< db->get_number_of_particles(v.second.second); ++j) {
          ParticleIndex decb(db->get_particle(v.second.second, j));
          double d= algebra::get_distance(m->get_sphere(deca),
                                          m->get_sphere(decb));
          if (d < dist) {
            if (!sink(deca, decb)) {
              return;
            }
          }
        }
      }
    } else if (da->get_is_leaf(v.second.first)) {
        for (unsigned int j=0;
             j< db->get_number_of_children(v.second.second); ++j) {
          unsigned int child = db->get_child(v.second.second, j);
          double d= distance_bound(m, da, v.second.first,
                                                     db, child);
          if (d < dist) {
            queue.push(QP(d, IP(v.second.first, child)));
          }
        }
    } else if (db->get_is_leaf(v.second.second)) {
      for (unsigned int i=0;
           i< da->get_number_of_children(v.second.first); ++i) {
        unsigned int child = da->get_child(v.second.first, i);
        double d= distance_bound(m, da, child,
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
          double d= distance_bound(m, da, childa,
                                   db, childb);
          if (d < dist) {
            queue.push(QP(d, IP(childa, childb)));
          }
        }
      }
    }
  }


  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    if (da->get_constituents().size()* db->get_constituents().size() < 1000) {
      for (unsigned int i=0; i< da->get_constituents().size(); ++i) {
        XYZR ca(m, da->get_constituents()[i]);
        for (unsigned int j=0; j< db->get_constituents().size(); ++j) {
          XYZR cb(m, db->get_constituents()[j]);
          if (get_distance(ca, cb) < .9*dist) {
            sink.check_contains(da->get_constituents()[i],
                                db->get_constituents()[j]);
          }
        }
      }
    }
  }
}


IMPCOREEXPORT
ParticlePairsTemp close_pairs(Model *m,
                              const RigidBodyHierarchy *da,
                              const RigidBodyHierarchy *db,
                              double dist);


template <class Sink>
inline void fill_close_particles(Model *m,
                                 const RigidBodyHierarchy *da,
                                 ParticleIndex pt, double dist,
                                 Sink sink) {
  typedef std::pair<double, int> QP;
  std::priority_queue<QP, base::Vector<QP>, LessFirst> queue;
  double d= distance_bound(m, da, 0, pt);
  if (d > dist) return;
  queue.push(QP(d, 0));
  do {
    std::pair<double, int> v= queue.top();
    queue.pop();
    if (da->get_is_leaf(v.second)) {
      for (unsigned int i=0; i< da->get_number_of_particles(v.second);
           ++i) {
        ParticleIndex p= da->get_particle(v.second, i);
        double d= algebra::get_distance(m->get_sphere(p),
                                        m->get_sphere(pt));
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
        double d= distance_bound(m, da, c, pt);
        if (d < dist) {
          queue.push(QP(d, c));
        }
      }
    }
  } while (!queue.empty());

  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< da->get_constituents().size(); ++i) {
      XYZR c(m, da->get_constituents()[i]);
      if (get_distance(c, XYZR(m, pt)) < .9*dist) {
        sink.check_contains(da->get_constituents()[i]);
      }
    }
  }
}

IMPCOREEXPORT
ParticlesTemp close_particles(Model *m,
                              const RigidBodyHierarchy *da,
                              XYZR pt, double dist);

IMPCOREEXPORT
RigidBodyHierarchy *get_rigid_body_hierarchy(RigidBody rb,
                                             ParticleIndexes constituents,
                                             ObjectKey mykey= ObjectKey());

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_RIGID_BODY_TREE_H */
