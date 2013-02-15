/**
 *  \file graph_base.cpp   \brief classes for implementing a graph.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/internal/rigid_body_tree.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/algebra/standard_grids.h>
#include <IMP/core/internal/grid_close_pairs_impl.h>
#include <IMP/utility.h>
#include <vector>

#include <typeinfo>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

static const double EXPANSION=1.0;
static const unsigned int MAX_LEAF_SIZE=10;


RigidBodyHierarchy::SpheresSplit
RigidBodyHierarchy::divide_spheres(const algebra::Sphere3Ds &ss,
                                   const SphereIndexes &s) {
  algebra::Vector3D centroid(0,0,0);
  for (unsigned int i=0; i< s.size(); ++i) {
    centroid += ss[s[i]].get_center();
  }
  centroid/= s.size();
  algebra::Vector3Ds pts(s.size());
  for (unsigned int i=0; i< s.size(); ++i) {
    pts[i]= ss[s[i]].get_center()-centroid;
  }
  algebra::PrincipalComponentAnalysis pca
    = algebra::get_principal_components(pts);
  algebra::Vector3D v0=pca.get_principal_component(0),
    v1= pca.get_principal_component(1),
    v2= pca.get_principal_component(2);
  double det = v0[0]*(v1[1]*v2[2]- v1[2]*v2[1])
    + v0[1]*(v1[2]*v2[0]-v1[0]*v2[2])
    + v0[2]*(v1[0]*v2[1]-v1[1]*v2[0]);
  if (det < 0) {
    v0= -v0;
  }
  algebra::Rotation3D r= algebra::get_rotation_from_matrix(v0[0], v0[1], v0[2],
                                                           v1[0], v1[1], v1[2],
                                                           v2[0], v2[1], v2[2])
    .get_inverse();
  algebra::Vector3D minc(std::numeric_limits<double>::max(),
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
  typedef algebra::DenseGrid3D<SphereIndexes> Grid;
  Grid grid(side, algebra::BoundingBox3D(minc, maxc), SphereIndexes());
  for (unsigned int i=0; i< s.size(); ++i) {
    Grid::Index ix= grid.get_nearest_index(pts[i]);
    grid[ix].push_back(s[i]);
    IMP_INTERNAL_CHECK(grid[ix].back() == s[i], "Failed to push");
  }

  SpheresSplit ret;
  for (Grid::AllIndexIterator it= grid.all_indexes_begin();
       it != grid.all_indexes_end(); ++it) {
    //std::cout << "Gathering from " << *it << std::endl;
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
                                       const ParticleIndexes &constituents):
    Object(d->get_name()+ " RigidBodyHierarchy %1%"), rb_(d),
  constituents_(constituents){
  IMP_IF_CHECK(base::USAGE) {
    ParticleIndexes uc=constituents;
    std::sort(uc.begin(), uc.end());
    uc.erase(std::unique(uc.begin(), uc.end()), uc.end());
    IMP_USAGE_CHECK(uc.size()== constituents.size(),
                    "Duplicate particles passed to rigid body tree: "
                    << constituents.size() << " > " << uc.size());
  }
  Model *m= d.get_model();
 // make sure children are up to date
  if (m->get_stage()== IMP::internal::NOT_EVALUATING) {
    d.set_reference_frame(d.get_reference_frame());
  }
  // they had better be up to date
  //d.update_members();
  std::sort(constituents_.begin(), constituents_.end());
  IMP_IF_CHECK(base::USAGE) {
    for (unsigned int i=0; i< constituents_.size(); ++i) {
      for (unsigned int j=0; j< 4; ++j) {
        IMP_USAGE_CHECK(m->get_has_attribute(FloatKey(j), constituents_[i]),
                        "Missing coordinates or radius");
      }
    }
  }
  set_was_used(true);
  IMP_LOG_TERSE( "Building rigid body hierarchy for particle "
          << d.get_particle()->get_name());
  if (rb_.get_members().size() < 100) {
    IMP_LOG_TERSE( " and particles " << rb_.get_members());
  }
  IMP_LOG_TERSE( std::endl);
  tree_.push_back(Data());
  // build spheres on internal coordinates
  IMP_USAGE_CHECK(constituents_.size() > 0,
                  "Rigid body has no members.");
  algebra::Sphere3Ds spheres(constituents_.size());
  for (unsigned int i=0; i< spheres.size(); ++i) {
    ParticleIndex rp= constituents_[i];
    double r =XYZR(m, rp).get_radius();
    algebra::Vector3D v= RigidMember(m, rp).get_internal_coordinates();
    spheres[i]= algebra::Sphere3D(v, r);
  }
  // call internal setup on spheres, 0, all indexes
  SphereIndexes leaves(spheres.size());
  for (unsigned int i=0; i< leaves.size(); ++i) {
    leaves[i]=i;
  }
  std::sort(leaves.begin(), leaves.end());

  base::Vector<Node> stack;
  stack.push_back(Node(0, leaves));

  do {
    Node cur;
    std::swap(cur,stack.back());
    stack.pop_back();
    build_tree(m, cur, spheres, stack);
  } while (!stack.empty());
  {
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      validate(m);
    }
  }
}


void RigidBodyHierarchy::build_tree(Model *m, const Node &cur,
                                    const algebra::Sphere3Ds &spheres,
                                    base::Vector<Node> &stack) {
  IMP_INTERNAL_CHECK(!cur.second.empty(), "Don't call me with no spheres");
  algebra::Sphere3Ds ss(cur.second.size());
  for (unsigned int i=0; i< cur.second.size(); ++i) {
    ss[i]= spheres[cur.second[i]];
  }
  algebra::Sphere3D ec= algebra::get_enclosing_sphere(ss);
  algebra::Sphere3D bs=algebra::Sphere3D(ec.get_center(),
                                         ec.get_radius()*EXPANSION
                                         +.1);
  set_sphere(cur.first, bs);
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< cur.second.size(); ++i) {
      algebra::Sphere3D bd(bs.get_center(), 1.1*bs.get_radius());
      IMP_INTERNAL_CHECK(bd.get_contains(spheres[cur.second[i]]),
                         "Sphere not contained " << bs
                           << " not around " << spheres[cur.second[i]]);
    }
  }
  if (cur.second.size() <MAX_LEAF_SIZE) {
    ParticleIndexes particles(cur.second.size());
    for (unsigned int i=0; i< particles.size(); ++i) {
      particles[i]= constituents_[cur.second[i]];
    }
    set_leaf(cur.first, particles);
    IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
        validate_internal(m, cur.first, algebra::Sphere3Ds());
    }
  } else {
    SpheresSplit ss= divide_spheres(spheres, cur.second);
    unsigned int nc= add_children(cur.first, ss.size());
    for (unsigned int i=0; i< ss.size(); ++i) {
      stack.push_back(Node(nc+i, ss[i]));
    }
  }
}

void RigidBodyHierarchy::set_sphere(unsigned int ni,
                                    const algebra::Sphere3D &s) {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  tree_[ni].s_=s;
}
void RigidBodyHierarchy::set_leaf(unsigned int ni,
                                  const ParticleIndexes &ids) {
  IMP_INTERNAL_CHECK(ni < tree_.size(), "Out of range");
  tree_[ni].children_.resize(ids.size());
  for (unsigned int i=0; i< ids.size(); ++i) {
    tree_[ni].children_[i]= -get_as_unsigned_int(ids[i])-1;
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



ParticleIndexes RigidBodyHierarchy::validate_internal(Model *m, int cur,
                                           algebra::Sphere3Ds bounds) const {
  bounds.push_back(algebra::Sphere3D(get_sphere(cur).get_center(),
                                     get_sphere(cur).get_radius()*1.1));
  if (get_is_leaf(cur)) {
    ParticleIndexes seen;
    for (unsigned int j=0; j< bounds.size(); ++j) {
      for (unsigned int i=0; i< get_number_of_particles(cur); ++i) {
        XYZR p(m, get_particle(cur, i));
        if (j==0) {
          IMP_INTERNAL_CHECK(std::find(seen.begin(), seen.end(),
                                       p.get_particle_index())==seen.end(),
                             "Particle " << p->get_name()
                             << " was seen more than once.");
          seen.push_back(get_particle(cur, i));
        }
        RigidMember rm(m, get_particle(cur, i));
        algebra::Vector3D ic= rm.get_internal_coordinates();
        algebra::ReferenceFrame3D rc=rb_.get_reference_frame();
        algebra::Sphere3D sc(rc.get_global_coordinates(ic),
                             p.get_radius());
        algebra::Sphere3D scg= p.get_sphere();
        IMP_INTERNAL_CHECK(algebra::get_distance(scg.get_center(),
                                                 sc.get_center()) < .1,
                           "Spheres do not match: " << sc << " vs " << scg
                           << " for particle "
                           << rm.get_particle()->get_name());
        IMP_INTERNAL_CHECK(bounds[j].get_contains(sc),
                           "Particle is not in bound " << p
                           << " has index " <<  get_particle(cur, i)
                           << " and internal coordinates "
                           << rm.get_internal_coordinates()
                           << " which map to "
                           << rb_.get_reference_frame()
                    .get_global_coordinates(rm.get_internal_coordinates())
                           << " bound " << bounds[j]
                           << " is " << j << " of " << bounds.size()
                           << " and untransformed sphere is "
                           << tree_[cur].s_);
      }
    }
    return seen;
  } else {
    ParticleIndexes seen;
    for (unsigned int i=0; i< get_number_of_children(cur); ++i) {
      int ci= get_child(cur, i);
      ParticleIndexes sc=validate_internal(m, ci, bounds);
      seen.insert(seen.end(), sc.begin(), sc.end());
    }
    return seen;
  }
}


void RigidBodyHierarchy::validate(Model *m) const {
  IMP_CHECK_OBJECT(this);
  ParticleIndexes all=validate_internal(m, 0, algebra::Sphere3Ds());
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    ParticleIndexes uall=all;
    std::sort(uall.begin(), uall.end());
    uall.erase(std::unique(uall.begin(), uall.end()), uall.end());
    IMP_INTERNAL_CHECK(all.size()==uall.size(),
                       "Duplicate entries were found: " << all.size()
                       << " != " << uall.size());
    IMP_INTERNAL_CHECK(all.size()==get_constituents().size(),
                       "Particle lists don't match in validate: "
                       << all.size() << "!=" << get_constituents().size());
  }
}



void RigidBodyHierarchy::do_show(std::ostream &out) const {
  for (unsigned int i=0; i< tree_.size(); ++i) {
    out << "Node " << i << ": ";
    if (get_is_leaf(i)) {
      for (unsigned int j=0; j< tree_[i].children_.size(); ++j) {
        out << get_particle(i, j) << " ";
      }
    } else {
      for (unsigned int j=0; j< tree_[i].children_.size(); ++j) {
        out << tree_[i].children_[j] << " ";
      }
    }
    out << ": " << tree_[i].s_ << std::endl;
  }
}

algebra::Sphere3Ds
RigidBodyHierarchy::get_tree() const {
  algebra::Sphere3Ds ret;
  for (unsigned int i=0; i< tree_.size(); ++i) {
    ret.push_back(get_sphere(i));
  }
  return ret;
}





Particle* closest_particle(Model *m, const RigidBodyHierarchy *da,
                           XYZR pt, double dist) {
  typedef std::pair<double, int> QP;
  std::priority_queue<QP, base::Vector<QP>, LessFirst> queue;
  double d= distance_bound(m, da, 0, pt.get_particle_index());
  queue.push(QP(d, 0));
  double best_d=dist;
  ParticleIndex bp=base::get_invalid_index<ParticleIndexTag>();
  do {
    std::pair<double, int> v= queue.top();
    queue.pop();
    if (v.first > best_d) break;
    if (da->get_is_leaf(v.second)) {
      for (unsigned int i=0; i< da->get_number_of_particles(v.second);
           ++i) {
        ParticleIndex p= da->get_particle(v.second, i);
        XYZR dd(m, p);
        double d= algebra::get_distance(m->get_sphere(p),
                                        m->get_sphere(pt.get_particle_index()));
        if (d < best_d) {
          best_d= d;
          bp= p;
        }
      }
    } else {
      for (unsigned int i=0; i< da->get_number_of_children(v.second);
           ++i) {
        unsigned int c= da->get_child(v.second, i);
        double d= distance_bound(m, da, c, pt.get_particle_index());
        if (d < best_d) {
          queue.push(QP(d, c));
        }
      }
    }
  } while (!queue.empty());
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    ParticleIndexes ps= da->get_constituents();
    for (unsigned int i=0; i< ps.size(); ++i) {
      XYZR d0(m, ps[i]);
      double d= get_distance(d0, pt);
      IMP_UNUSED(d);
      IMP_INTERNAL_CHECK(d>.9*best_d,
                         "Missed the particle: " << d0
                         << " for " << pt << " at " << d
                         << " vs " << best_d << " for " << bp);
    }
  }
  return m->get_particle(bp);
}




ParticlePair closest_pair(Model *m, const RigidBodyHierarchy *da,
                          const RigidBodyHierarchy *db,
                          double dist) {
  typedef std::pair<int,int> IP;
  typedef std::pair<double, IP> QP;
  std::priority_queue<QP, base::Vector<QP>, LessFirst> queue;
  double d= distance_bound(m, da, 0, db, 0);
  queue.push(QP(d, IP(0,0)));
  double best_d=dist;
  ParticlePair bp;
  do {
    QP v= queue.top();
    queue.pop();
    if (v.first > best_d) break;
    /*IMP_LOG_TERSE( "Trying pair " << v.second.first << " " << v.second.second
      << std::endl);*/
    if (da->get_is_leaf(v.second.first) && db->get_is_leaf(v.second.second)) {
      for (unsigned int i=0;
           i< da->get_number_of_particles(v.second.first); ++i) {
        XYZR deca(m, da->get_particle(v.second.first, i));
        for (unsigned int j=0;
             j< db->get_number_of_particles(v.second.second); ++j) {
          XYZR decb(m, db->get_particle(v.second.second, j));
          double d= get_distance(deca, decb);
          if (d < best_d) {
            bp= ParticlePair(deca, decb);
            /*IMP_LOG_VERBOSE( "Updating threshold to " << best_d
              << " due to pair " << bp << std::endl);*/
            best_d= d;
            /*std::cout << "Updating threshold to " << best_d
              << " due to pair " << bp << std::endl;*/
          }
        }
      }
    } else if (da->get_is_leaf(v.second.first)) {
        for (unsigned int j=0;
             j< db->get_number_of_children(v.second.second); ++j) {
          unsigned int child = db->get_child(v.second.second, j);
          double d= distance_bound(m, da, v.second.first,
                                                     db, child);
          if (d < best_d) {
            queue.push(QP(d, IP(v.second.first, child)));
          }
        }
    } else if (db->get_is_leaf(v.second.second)) {
      for (unsigned int i=0;
           i< da->get_number_of_children(v.second.first); ++i) {
        unsigned int child = da->get_child(v.second.first, i);
        double d= distance_bound(m, da, child,
                                   db, v.second.second);
          if (d < best_d) {
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
          if (d < best_d) {
            queue.push(QP(d, IP(childa, childb)));
          }
        }
      }
    }
  } while (!queue.empty());
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    ParticleIndexes psa= da->get_constituents();
    ParticleIndexes psb= db->get_constituents();
    for (unsigned int i=0; i< psa.size(); ++i) {
      for (unsigned int j=0; j< psb.size(); ++j) {
        XYZR d0a(m, psa[i]);
        XYZR d0b(m, psb[j]);
        double d= get_distance(d0a, d0b);
        IMP_UNUSED(d);
        IMP_INTERNAL_CHECK(d > .9*best_d,
                           "Missed the pair: " << d0a << " and " << d0b
                           << " at " << d << " vs "
                           << XYZR(bp[0]) << " and " << XYZR(bp[1])
                           << " at " << best_d);
      }
    }
  }
  return bp;
}



RigidBodyHierarchy *get_rigid_body_hierarchy(RigidBody rb,
                                             ParticleIndexes constituents,
                                             ObjectKey mykey) {
  IMP_LOG_VERBOSE( "Fetching hierarchy from " << rb->get_name()
          << " (" << mykey << ")" << std::endl);
  static ObjectKeys keys;
  if (mykey!=ObjectKey() && rb->has_attribute(mykey)) {
    RigidBodyHierarchy*ret=
      base::object_cast<RigidBodyHierarchy>(rb->get_value(mykey));
    IMP_INTERNAL_CHECK(ret, "No hierarchy found");
    IMP_LOG_VERBOSE( "Cached" << std::endl);
    return ret;
  }
  std::sort(constituents.begin(), constituents.end());
  ObjectKey free;
  for (unsigned int i=0; i< keys.size(); ++i) {
    if (rb->has_attribute(keys[i])) {
      Pointer<RigidBodyHierarchy> cur
        =base::object_cast<RigidBodyHierarchy>(rb->get_value(keys[i]));
      IMP_CHECK_OBJECT(cur);
      if (cur->get_constituents_match(constituents)) {
        if (mykey != ObjectKey()) {
          rb.get_model()->add_cache_attribute(mykey,
                                              rb.get_particle_index(),
                                              cur.get());
        }
        IMP_CHECK_OBJECT(cur);
        IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
          cur->validate(rb.get_model());
        }
        if (mykey != ObjectKey()) {
          IMP_LOG_TERSE( "Storing tree at " << mykey << std::endl);
          rb.get_model()->add_cache_attribute(mykey,
                                              rb.get_particle_index(),
                                              cur.get());
        }
        IMP_IF_CHECK( base::USAGE_AND_INTERNAL) {
          cur->validate(rb.get_model());
        }
        return cur;
      }
    } else if (free== ObjectKey()) {
      free=keys[i];
      break;
    }
  }
  if (free==ObjectKey()) {
    std::ostringstream oss;
    oss << "RB Hierarchy " << keys.size();
    keys.push_back(ObjectKey(oss.str()));
    free=keys.back();
    add_rigid_body_cache_key(keys.back());
  }
  Pointer<RigidBodyHierarchy> h= new RigidBodyHierarchy(rb, constituents);
  if (mykey != ObjectKey()) {
    IMP_LOG_TERSE( "Storing tree at " << mykey << std::endl);
    rb.get_model()->add_cache_attribute(mykey,
                                        rb.get_particle_index(),
                                        h.get());
  }
  IMP_CHECK_OBJECT(h);
  IMP_IF_CHECK( base::USAGE_AND_INTERNAL) {
    h->validate(rb.get_model());
  }
  return h;
}


ParticlePairsTemp close_pairs(Model *m,
                              const RigidBodyHierarchy *da,
                              const RigidBodyHierarchy *db,
                              double dist) {
  ParticlePairsTemp ret;
  fill_close_pairs(m, da, db, dist, ParticlePairSink(m,
                                                     PairPredicates(),
                                                     ret));
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    std::sort(ret.begin(), ret.end());
    ParticleIndexes psa= da->get_constituents();
    ParticleIndexes psb= db->get_constituents();
    for (unsigned int i=0; i< psa.size(); ++i) {
      for (unsigned int j=0; j< psb.size(); ++j) {
        XYZR d0a(m, psa[i]);
        XYZR d0b(m, psb[j]);
        double d= get_distance(d0a, d0b);
        if (d < dist) {
          IMP_INTERNAL_CHECK(std::binary_search(ret.begin(), ret.end(),
                                                ParticlePair(d0a, d0b)),
                           "Missed a pair: " << d0a << " and " << d0b
                           << " at " << d << " vs "
                           << dist);
        }
      }
    }
  }
  return ret;
}


ParticlesTemp close_particles(Model *m,
                              const RigidBodyHierarchy *da,
                              XYZR pt, double dist) {
  ParticlesTemp ret;
  fill_close_particles(m, da, pt.get_particle_index(),
                       dist, ParticleSink(m, ret));
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    std::sort(ret.begin(), ret.end());
    ParticleIndexes ps= da->get_constituents();
    for (unsigned int i=0; i< ps.size(); ++i) {
      XYZR d0(m, ps[i]);
      double d= get_distance(d0, pt);
      if (d < .9*dist) {
        IMP_INTERNAL_CHECK(std::binary_search(ret.begin(), ret.end(),
                                              m->get_particle(ps[i])),
                           "Missed a particle: " << d0
                           << " for " << pt << " at " << d);
      }
    }
  }
  return ret;
}


IMPCORE_END_INTERNAL_NAMESPACE
