/**
 *  \file graph_base.cpp   \brief classes for implementing a graph.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/internal/rigid_body_tree.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/algebra/Grid3D.h>
#include <typeinfo>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

static const double EXPANSION=1.0;
static const unsigned int MAX_LEAF_SIZE=10;


RigidBodyHierarchy::SpheresSplit
RigidBodyHierarchy::divide_spheres(const std::vector<algebra::SphereD<3> > &ss,
                                   const SphereIndexes &s) {
  algebra::VectorD<3> centroid(0,0,0);
  for (unsigned int i=0; i< s.size(); ++i) {
    centroid += ss[s[i]].get_center();
  }
  centroid/= s.size();
  std::vector<algebra::VectorD<3> > pts(s.size());
  for (unsigned int i=0; i< s.size(); ++i) {
    pts[i]= ss[s[i]].get_center()-centroid;
  }
  algebra::PrincipalComponentAnalysis pca
    = algebra::get_principal_components(pts);
  const algebra::VectorD<3> &v0=pca.get_principal_component(0),
    &v1= pca.get_principal_component(1),
    &v2= pca.get_principal_component(2);
  algebra::Rotation3D r= algebra::get_rotation_from_matrix(v0[0], v0[1], v0[2],
                                                       v1[0], v1[1], v1[2],
                                                       v2[0], v2[1], v2[2])
    .get_inverse();
  algebra::VectorD<3> minc(std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max()),
    maxc(-std::numeric_limits<double>::max(),
         -std::numeric_limits<double>::max(),
         -std::numeric_limits<double>::max());
  for (unsigned int i=0; i< s.size(); ++i) {
    pts[i]= r.get_rotated(pts[i]);
    for (unsigned int j=0; j< 3; ++j) {
      minc[j]= std::min(minc[j], pts[i][j]);
      maxc[j]= std::max(maxc[j], pts[i][j]);
    }
  }
  double side=0;
  for (unsigned int j=0; j< 3; ++j) {
    side= std::max(side, (maxc[j]-minc[j])/2.0);
  }
  typedef algebra::Grid3D<SphereIndexes > Grid;
  Grid grid(side, algebra::BoundingBox3D(minc, maxc), SphereIndexes());
  for (unsigned int i=0; i< s.size(); ++i) {
    Grid::Index ix= grid.get_index(pts[i]);
    grid[ix].push_back(s[i]);
    IMP_INTERNAL_CHECK(grid[ix].back() == s[i], "Failed to push");
  }

  SpheresSplit ret;
  for (Grid::IndexIterator it= grid.all_indexes_begin();
       it != grid.all_indexes_end(); ++it) {
    std::cout << "Gathering from " << *it << std::endl;
    if (!grid[*it].empty()) {
      ret.push_back(grid[*it]);
    }
  }
  return ret;
}



/* create a tree in a vector where the stored data is
   - indexes of children
   - bounding sphere (in the rigid body internal frame)

   a node always has children, even it is a leaf (if it is a leaf, the child
   is itself). Encode being a leaf by having a negative last index, that being
   the index into the array of particles.
*/
RigidBodyHierarchy::RigidBodyHierarchy(RigidBody d,
                                       Refiner *r): rb_(d), r_(r) {
  IMP_LOG(TERSE, "Building rigid body hierarchy for particle "
          << d.get_particle()->get_name() << " with refiner "
          << r->get_name() << " and particles " << Particles(r->get_refined(d))
          << std::endl);
  tree_.push_back(Data());
  set_name(std::string("Rigid body hierachy for particle "
                       + d.get_particle()->get_name()));
  // build spheres on internal coordinates
  IMP_USAGE_CHECK(r->get_number_of_refined(d) > 0,
                  "Refiner cannot produce any particles for rigid body.");
  std::vector<algebra::SphereD<3> > spheres(r->get_number_of_refined(d));
  for (unsigned int i=0; i< spheres.size(); ++i) {
    Particle *rp= r->get_refined(d, i);
    double r =XYZR(rp).get_radius();
    algebra::VectorD<3> v= RigidMember(rp).get_internal_coordinates();
    // make sure they are relative to this rigid body
    // I should check that they are connected, too lazy
    if (RigidMember(rp).get_rigid_body()== d) {
    } else {
      v= RigidMember(d).get_internal_transformation()
        .get_inverse().get_transformed(v);
    }
    spheres[i]= algebra::SphereD<3>(v, r);
  }
  // call internal setup on spheres, 0, all indexes
  SphereIndexes leaves(spheres.size());
  for (unsigned int i=0; i< leaves.size(); ++i) {
    leaves[i]=i;
  }

  typedef std::pair<unsigned int, SphereIndexes> Node;
  std::vector<Node> stack;
  stack.push_back(Node(0, leaves));

  do {
    Node cur;
    std::swap(cur,stack.back());
    stack.pop_back();
    IMP_INTERNAL_CHECK(!cur.second.empty(), "Don't call me with no spheres");
    std::vector<algebra::SphereD<3> > ss(cur.second.size());
    for (unsigned int i=0; i< cur.second.size(); ++i) {
      ss[i]= spheres[cur.second[i]];
    }
    algebra::SphereD<3> ec= algebra::get_enclosing_sphere(ss);
    set_sphere(cur.first, algebra::SphereD<3>(ec.get_center(),
                                            ec.get_radius()*EXPANSION));
    if (cur.second.size() <MAX_LEAF_SIZE) {
      set_leaf(cur.first, cur.second);
    } else {
      SpheresSplit ss= divide_spheres(spheres, cur.second);
      unsigned int nc= add_children(cur.first, ss.size());
      for (unsigned int i=0; i< ss.size(); ++i) {
        stack.push_back(Node(nc+i, ss[i]));
      }
    }
  } while (!stack.empty());
}


void RigidBodyHierarchy::set_sphere(unsigned int ni,
                                    const algebra::SphereD<3> &s) {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  tree_[ni].s_=s;
}
void RigidBodyHierarchy::set_leaf(unsigned int ni,
                                  const std::vector<unsigned int> &ids) {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  tree_[ni].children_.resize(ids.size());
  for (unsigned int i=0; i< ids.size(); ++i) {
    tree_[ni].children_[i]= -ids[i]-1;
  }
}
unsigned int RigidBodyHierarchy::add_children(unsigned int ni,
                                              unsigned int num_children)  {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  IMP_INTERNAL_CHECK(num_children >1, "Need to have children");
  unsigned int ret= tree_.size();
  tree_.insert(tree_.end(), num_children, Data());
  tree_[ni].children_.resize(num_children);
  for (unsigned int i=0; i< num_children; ++i) {
    tree_[ni].children_[i]= ret+i;
  }
  return ret;
}
bool RigidBodyHierarchy::get_is_leaf(unsigned int ni) const {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  IMP_INTERNAL_CHECK(!tree_[ni].children_.empty(),
             "Everything must have particles or children");
  return tree_[ni].children_[0] < 0;
}
unsigned int
RigidBodyHierarchy::get_number_of_particles(unsigned int ni) const  {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  IMP_INTERNAL_CHECK(get_is_leaf(ni), "Only leaves have particles");
  return tree_[ni].children_.size();
}
unsigned int
RigidBodyHierarchy::get_number_of_children(unsigned int ni) const  {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  if (!get_is_leaf(ni)) {
    return tree_[ni].children_.size();
  } else {
    return 1;
  }
}
unsigned int RigidBodyHierarchy::get_child(unsigned int ni,
                                           unsigned int i) const  {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  IMP_INTERNAL_CHECK(tree_[ni].children_.size() > i,
             "Out of range in particle");
  if (!get_is_leaf(ni)) {
    return tree_[ni].children_[i];
  } else {
    return ni;
  }
}


Particle* RigidBodyHierarchy::get_particle(unsigned int ni,
                                           unsigned int i) const  {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  IMP_INTERNAL_CHECK(tree_[ni].children_.size() > i,
             "Out of range in particle");
  IMP_INTERNAL_CHECK(tree_[ni].children_[i] < 0,
             "Not a leaf node");
  int index= std::abs(tree_[ni].children_[i])-1;
  return r_->get_refined(rb_, index);
}

ParticlesTemp RigidBodyHierarchy::get_particles(unsigned int i) const {
  ParticlesTemp ret;
  std::vector<int> stack(1, i);
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



void RigidBodyHierarchy::do_show(std::ostream &out) const {
  for (unsigned int i=0; i< tree_.size(); ++i) {
    out << "Node " << i << ": ";
    if (get_is_leaf(i)) {
      for (unsigned int j=0; j< tree_[i].children_.size(); ++j) {
        out << get_particle(i, j)->get_name() << " ";
      }
    } else {
      for (unsigned int j=0; j< tree_[i].children_.size(); ++j) {
        out << tree_[i].children_[j] << " ";
      }
    }
    out << ": " << tree_[i].s_ << std::endl;
  }
}

std::vector<algebra::SphereD<3> >
RigidBodyHierarchy::get_tree() const {
  std::vector<algebra::SphereD<3> > ret;
  for (unsigned int i=0; i< tree_.size(); ++i) {
    ret.push_back(get_sphere(i));
  }
  return ret;
}

ObjectKey get_rigid_body_hierarchy_key(Refiner *r) {
  ObjectKey ok(std::string("rigid body hierachy ")+typeid(r).name());
  return ok;
}











namespace {
inline double
distance_bound(XYZR a, XYZR b) {
  return get_distance(a,b);
}

inline double
distance_bound(const RigidBodyHierarchy *da, unsigned int i,
               XYZR b) {
  algebra::SphereD<3> s= da->get_sphere(i);
  double rd= algebra::get_distance(s, b.get_sphere());
#if 0
  ParticlesTemp ta= da->get_particles(i);

  for (unsigned int ii=0; ii< ta.size(); ++ii) {
    double td= distance(XYZR(ta[ii]).get_sphere(),b.get_sphere());
    IMP_INTERNAL_CHECK(td >= .95*rd, "Error in bounds");
  }
#endif
  return rd;
}

inline double
distance_bound(const RigidBodyHierarchy *da, unsigned int i,
               const RigidBodyHierarchy *db, unsigned int j) {
  algebra::SphereD<3> sa= da->get_sphere(i);
  algebra::SphereD<3> sb= db->get_sphere(j);
  double rd= algebra::get_distance(sa, sb);
#if 0
  ParticlesTemp ta= da->get_particles(i);
  ParticlesTemp tb= db->get_particles(j);

  for (unsigned int ii=0; ii< ta.size(); ++ii) {
    for (unsigned int ij=0; ij< tb.size(); ++ij) {
      double td= distance(XYZR(ta[ii]), XYZR(tb[ij]));
      IMP_INTERNAL_CHECK(td >= .95*rd, "Error in bounds "
                 << " the pair " << ParticlePair(ta[ii], tb[ij])
                 << " has distance " << td << " but the sphere "
                 << " bound is " << rd << ". Spheres are "
                 << sa << " and " << sb
                 << " and particles are " << XYZR(ta[ii])
                 << " and " << XYZR(tb[ij]));
    }
  }
#endif
  return rd;
}

}

Particle* closest_particle(const RigidBodyHierarchy *da,
                                  XYZR pt) {
  typedef std::pair<double, int> QP;
  std::multimap<double, int> queue;
  double d= distance_bound(da, 0, pt);
  queue.insert(QP(d, 0));
  double best_d=std::numeric_limits<double>::max();
  Particle *bp=NULL;
  do {
    std::pair<double, int> v= *queue.begin();
    queue.erase(queue.begin());
    if (v.first > best_d) break;
    if (da->get_is_leaf(v.second)) {
      for (unsigned int i=0; i< da->get_number_of_particles(v.second);
           ++i) {
        Particle *p= da->get_particle(v.second, i);
        XYZR dd(p);
        double d= distance_bound(dd, pt);
        if (d < best_d) {
          best_d= d;
          bp= p;
        }
      }
    } else {
      for (unsigned int i=0; i< da->get_number_of_children(v.second);
           ++i) {
        unsigned int c= da->get_child(v.second, i);
        double d= distance_bound(da, c, pt);
        if (d < best_d) {
          queue.insert(QP(d, c));
        }
      }
    }
  } while (!queue.empty());
  return bp;
}


ParticlePair closest_pair(const RigidBodyHierarchy *da,
                                 const RigidBodyHierarchy *db) {
  typedef std::pair<int,int> IP;
  typedef std::pair<double, IP> QP;
  std::multimap<double, IP> queue;
  double d= distance_bound(da, 0, db, 0);
  queue.insert(QP(d, IP(0,0)));
  double best_d=std::numeric_limits<double>::max();
  ParticlePair bp;
  do {
    QP v= *queue.begin();
    queue.erase(queue.begin());
    if (v.first > best_d) break;
    /*IMP_LOG(TERSE, "Trying pair " << v.second.first << " " << v.second.second
      << std::endl);*/
    if (da->get_is_leaf(v.second.first) && db->get_is_leaf(v.second.second)) {
      for (unsigned int i=0;
           i< da->get_number_of_particles(v.second.first); ++i) {
        for (unsigned int j=0;
             j< db->get_number_of_particles(v.second.second); ++j) {
          XYZR deca(da->get_particle(v.second.first, i));
          XYZR decb(db->get_particle(v.second.second, j));
          double d= get_distance(deca, decb);
          if (d < best_d) {
            bp= ParticlePair(deca, decb);
            /*IMP_LOG(VERBOSE, "Updating threshold to " << best_d
              << " due to pair " << bp << std::endl);*/
            best_d= d;
          }
        }
      }
    } else if (da->get_is_leaf(v.second.first)) {
        for (unsigned int j=0;
             j< db->get_number_of_children(v.second.second); ++j) {
          unsigned int child = db->get_child(v.second.second, j);
          double d= distance_bound(da, v.second.first,
                                                     db, child);
          if (d < best_d) {
            queue.insert(QP(d, IP(v.second.first, child)));
          }
        }
    } else if (db->get_is_leaf(v.second.second)) {
      for (unsigned int i=0;
           i< da->get_number_of_children(v.second.first); ++i) {
        unsigned int child = da->get_child(v.second.first, i);
          double d= distance_bound(da, child,
                                   db, v.second.second);
          if (d < best_d) {
            queue.insert(QP(d, IP(child, v.second.second)));
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
          if (d < best_d) {
            queue.insert(QP(d, IP(childa, childb)));
          }
        }
      }
    }
  } while (!queue.empty());
  return bp;
}

IMPCORE_END_INTERNAL_NAMESPACE
