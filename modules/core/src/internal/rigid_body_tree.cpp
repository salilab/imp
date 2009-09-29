/**
 *  \file graph_base.cpp   \brief classes for implementing a graph.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/internal/rigid_body_tree.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/core/internal/Grid3D.h>
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
  algebra::PrincipalComponentAnalysis pca= algebra::principal_components(pts);
  const algebra::Vector3D &v0=pca.get_principal_component(0),
    &v1= pca.get_principal_component(1),
    &v2= pca.get_principal_component(2);
  algebra::Rotation3D r= algebra::rotation_from_matrix(v0[0], v0[1], v0[2],
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
    pts[i]= r.rotate(pts[i]);
    for (unsigned int j=0; j< 3; ++j) {
      minc[j]= std::min(minc[j], pts[i][j]);
      maxc[j]= std::max(maxc[j], pts[i][j]);
    }
  }
  double side=0;
  for (unsigned int j=0; j< 3; ++j) {
    side= std::max(side, (maxc[j]-minc[j])/2.0);
  }
  typedef Grid3D<SphereIndexes > Grid;
  Grid grid(side, minc, maxc, SphereIndexes());
  for (unsigned int i=0; i< s.size(); ++i) {
    Grid::Index ix= grid.get_index(pts[i]);
    grid.get_voxel(ix).push_back(s[i]);
  }

  SpheresSplit ret;
  for (Grid::IndexIterator it= grid.all_indexes_begin();
       it != grid.all_indexes_end(); ++it) {
    if (!grid.get_voxel(*it).empty()) {
      ret.push_back(grid.get_voxel(*it));
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
          << r->get_name() << std::endl);
  tree_.push_back(Data());
  set_name(std::string("Rigid body hierachy for particle "
                       + d.get_particle()->get_name()));
  // build spheres on internal coordinates
  algebra::Sphere3Ds spheres(r->get_number_of_refined(d));
  for (unsigned int i=0; i< spheres.size(); ++i) {
    Particle *rp= r->get_refined(d, i);
    double r =XYZR(rp).get_radius();
    algebra::Vector3D v= RigidMember(rp).get_internal_coordinates();
    spheres[i]= algebra::Sphere3D(v, r);
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
    IMP_assert(!cur.second.empty(), "Don't call me with no spheres");
    algebra::Sphere3Ds ss(cur.second.size());
    for (unsigned int i=0; i< cur.second.size(); ++i) {
      ss[i]= spheres[cur.second[i]];
    }
    algebra::Sphere3D ec= algebra::enclosing_sphere(ss);
    set_sphere(cur.first, algebra::Sphere3D(ec.get_center(),
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
                                    const algebra::Sphere3D &s) {
  IMP_assert(ni < tree_.size(), "Out of range");
  tree_[ni].s_=s;
}
void RigidBodyHierarchy::set_leaf(unsigned int ni,
                                  const std::vector<unsigned int> &ids) {
  IMP_assert(ni < tree_.size(), "Out of range");
  tree_[ni].children_.resize(ids.size());
  for (unsigned int i=0; i< ids.size(); ++i) {
    tree_[ni].children_[i]= -ids[i]-1;
  }
}
unsigned int RigidBodyHierarchy::add_children(unsigned int ni,
                                              unsigned int num_children)  {
  IMP_assert(ni < tree_.size(), "Out of range");
  IMP_assert(num_children >1, "Need to have children");
  unsigned int ret= tree_.size();
  tree_.insert(tree_.end(), num_children, Data());
  tree_[ni].children_.resize(num_children);
  for (unsigned int i=0; i< num_children; ++i) {
    tree_[ni].children_[i]= ret+i;
  }
  return ret;
}
bool RigidBodyHierarchy::get_is_leaf(unsigned int ni) const {
  IMP_assert(ni < tree_.size(), "Out of range");
  IMP_assert(!tree_[ni].children_.empty(),
             "Everything must have particles or children");
  return tree_[ni].children_[0] < 0;
}
unsigned int
RigidBodyHierarchy::get_number_of_particles(unsigned int ni) const  {
  IMP_assert(ni < tree_.size(), "Out of range");
  IMP_assert(get_is_leaf(ni), "Only leaves have particles");
  return tree_[ni].children_.size();
}
unsigned int
RigidBodyHierarchy::get_number_of_children(unsigned int ni) const  {
  IMP_assert(ni < tree_.size(), "Out of range");
  if (!get_is_leaf(ni)) {
    return tree_[ni].children_.size();
  } else {
    return 1;
  }
}
unsigned int RigidBodyHierarchy::get_child(unsigned int ni,
                                           unsigned int i) const  {
  IMP_assert(ni < tree_.size(), "Out of range");
  IMP_assert(tree_[ni].children_.size() > i,
             "Out of range in particle");
  if (!get_is_leaf(ni)) {
    return tree_[ni].children_[i];
  } else {
    return ni;
  }
}


Particle* RigidBodyHierarchy::get_particle(unsigned int ni,
                                           unsigned int i) const  {
  IMP_assert(ni < tree_.size(), "Out of range");
  IMP_assert(tree_[ni].children_.size() > i,
             "Out of range in particle");
  IMP_assert(tree_[ni].children_[i] < 0,
             "Not a leaf node");
  int index= std::abs(tree_[ni].children_[i])-1;
  return r_->get_refined(rb_, index);
}



void RigidBodyHierarchy::show(std::ostream &out) const {
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

std::vector<algebra::Sphere3D>
RigidBodyHierarchy::get_tree() const {
  std::vector<algebra::Sphere3D> ret;
  for (unsigned int i=0; i< tree_.size(); ++i) {
    ret.push_back(get_sphere(i));
  }
  return ret;
}

ObjectKey get_rigid_body_hierarchy_key(Refiner *r) {
  ObjectKey ok(std::string("rigid body hierachy ")+typeid(r).name());
  return ok;
}

IMPCORE_END_INTERNAL_NAMESPACE
