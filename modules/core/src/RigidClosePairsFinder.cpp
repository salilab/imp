/**
 *  \file RigidClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/RigidClosePairsFinder.h"
#include "IMP/core/BoxSweepClosePairsFinder.h"
#include "IMP/core/GridClosePairsFinder.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/core/XYZ.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/core/internal/Grid3D.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

static const double EXPANSION=1.0;
static const unsigned int MAX_LEAF_SIZE=10;

RigidClosePairsFinder::RigidClosePairsFinder():
  cpfout_(new ListPairContainer()){
  #ifdef IMP_USE_CGAL
  cpf_=Pointer<ClosePairsFinder>(new BoxSweepClosePairsFinder());
#else
  cpf_=Pointer<ClosePairsFinder>(new GridClosePairsFinder());
#endif
}
RigidClosePairsFinder
::RigidClosePairsFinder(ClosePairsFinder *cpf): cpf_(cpf),
                        cpfout_(new ListPairContainer()){}

RigidClosePairsFinder::~RigidClosePairsFinder(){}



namespace {
  internal::SpheresSplit divide_spheres(const algebra::Sphere3Ds &ss,
                                        const internal::SphereIndexes &s) {
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
    typedef internal::Grid3D<internal::SphereIndexes > Grid;
    Grid grid(side, minc, maxc, internal::SphereIndexes());
    for (unsigned int i=0; i< s.size(); ++i) {
      Grid::Index ix= grid.get_index(pts[i]);
      grid.get_voxel(ix).push_back(s[i]);
    }

    internal::SpheresSplit ret;
    for (Grid::IndexIterator it= grid.all_indexes_begin();
         it != grid.all_indexes_end(); ++it) {
      if (!grid.get_voxel(*it).empty()) {
        ret.push_back(grid.get_voxel(*it));
      }
    }
    return ret;
  }
}


namespace internal {
  RigidBodyParticleData::RigidBodyParticleData() {
    data_.push_back(Data());
  }
  void RigidBodyParticleData::set_sphere(unsigned int ni,
                                         const algebra::Sphere3D &s) {
    IMP_assert(ni < data_.size(), "Out of range");
    data_[ni].s_=s;
  }
  void RigidBodyParticleData::set_leaf(unsigned int ni,
                                       const std::vector<unsigned int> &ids) {
    IMP_assert(ni < data_.size(), "Out of range");
    data_[ni].storage_.resize(ids.size());
    for (unsigned int i=0; i< ids.size(); ++i) {
      data_[ni].storage_[i]= -ids[i]-1;
    }
  }
  unsigned int RigidBodyParticleData::add_children(unsigned int ni,
                                                   unsigned int num_children)  {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(num_children >1, "Need to have children");
    unsigned int ret= data_.size();
    data_.insert(data_.end(), num_children, Data());
    data_[ni].storage_.resize(num_children);
    for (unsigned int i=0; i< num_children; ++i) {
      data_[ni].storage_[i]= ret+i;
    }
    return ret;
  }
  bool RigidBodyParticleData::get_is_leaf(unsigned int ni) const {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(!data_[ni].storage_.empty(),
               "Everything must have particles or children");
    return data_[ni].storage_[0] < 0;
  }
  unsigned int
  RigidBodyParticleData::get_number_of_particles(unsigned int ni) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(get_is_leaf(ni), "Only leaves have particles");
    return data_[ni].storage_.size();
  }
  unsigned int
  RigidBodyParticleData::get_number_of_children(unsigned int ni) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    if (!get_is_leaf(ni)) {
      return data_[ni].storage_.size();
    } else {
      return 1;
    }
  }
  unsigned int RigidBodyParticleData::get_child(unsigned int ni,
                                                unsigned int i) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(data_[ni].storage_.size() > i,
               "Out of range in particle");
    if (!get_is_leaf(ni)) {
      return data_[ni].storage_[i];
    } else {
      return ni;
    }
  }
  Particle* RigidBodyParticleData::get_particle(unsigned int ni,
                                                   unsigned int i,
                                                   Particle *p) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    IMP_assert(data_[ni].storage_.size() > i,
               "Out of range in particle");
    IMP_assert(data_[ni].storage_[i] < 0,
               "Not a leaf node");
    if (!RigidBody::is_instance_of(p)) return p;
    int index= std::abs(data_[ni].storage_[i])-1;
    return RigidBody(p).get_member(index).get_particle();
  }
  const algebra::Sphere3D &
  RigidBodyParticleData::get_sphere(unsigned int ni) const  {
    IMP_assert(ni < data_.size(), "Out of range");
    return data_[ni].s_;
  }

  void RigidBodyParticleData::show_tree(std::ostream &out, Particle *p) const {
    for (unsigned int i=0; i< data_.size(); ++i) {
      out << "Node " << i << ": ";
      if (get_is_leaf(i)) {
        for (unsigned int j=0; j< data_[i].storage_.size(); ++j) {
          out << get_particle(i, j, p)->get_name() << " ";
        }
      } else {
        for (unsigned int j=0; j< data_[i].storage_.size(); ++j) {
          out << data_[i].storage_[j] << " ";
        }
      }
      out << ": " << data_[i].s_ << std::endl;
    }
  }

  std::vector<algebra::Sphere3D>
  RigidBodyParticleData::get_spheres() const {
    std::vector<algebra::Sphere3D> ret;
    for (unsigned int i=0; i< data_.size(); ++i) {
      ret.push_back(get_sphere(i));
    }
    return ret;
  }

  void RigidBodyCollisionData::add_data(Particle *p,
                                        const algebra::Sphere3D &s) {
    IMP_assert(data_.find(p) == data_.end(),
               "Already have data for " << p->get_name());
    data_[p]= RigidBodyParticleData();
    data_[p].set_sphere(0, s);
    data_[p].set_leaf(0, std::vector<unsigned int>(1,0));
  }
  void RigidBodyCollisionData::add_data(Particle *p) {
    data_[p]= RigidBodyParticleData();
  }
  RigidBodyParticleData &RigidBodyCollisionData::get_data(Particle *p) {
    IMP_assert(data_.find(p) != data_.end(),
               "Internal error, not set up for particle "
               << p->get_name());
    return data_.find(p)->second;
  }
  const RigidBodyParticleData &
  RigidBodyCollisionData::get_data(Particle *p) const {
    IMP_assert(data_.find(p) != data_.end(),
               "Internal error, not set up for particle "
               << p->get_name());
    return data_.find(p)->second;
  }
  bool RigidBodyCollisionData::has_data(Particle *p) const {
    return data_.find(p) != data_.end();
  }
}

const algebra::Sphere3D
RigidClosePairsFinder::get_transformed(Particle *a,
                                     const algebra::Sphere3D &s) const {
  if (RigidBody::is_instance_of(a)) {
    RigidBody d(a);
    return algebra::Sphere3D(d.get_transformation().transform(s.get_center()),
                             s.get_radius());
  } else {
    XYZ d(a);
    return algebra::Sphere3D(d.get_coordinates(),
                             get_radius(a)+get_distance()*.5);
  }
}

Particle *RigidClosePairsFinder::get_member(Particle *a,
                                          unsigned int i) const {
   if (RigidBody::is_instance_of(a)) {
     RigidBody d(a);
     return d.get_member(i).get_particle();
   } else {
     return a;
   }
}

void
RigidClosePairsFinder::setup(const algebra::Sphere3Ds &spheres,
                           unsigned int node_index,
                           const internal::SphereIndexes &leaves,
                           internal::RigidBodyParticleData &data) const {
  // compute sphere for root
  // could make more efficient
  IMP_assert(!leaves.empty(), "Don't call me with no spheres");
  algebra::Sphere3Ds ss(leaves.size());
  for (unsigned int i=0; i< leaves.size(); ++i) {
    ss[i]= spheres[leaves[i]];
  }
  algebra::Sphere3D ec= algebra::enclosing_sphere(ss);
  data.set_sphere(node_index, algebra::Sphere3D(ec.get_center(),
                        (ec.get_radius()+get_distance()/2.0)*EXPANSION));
  if (leaves.size() <MAX_LEAF_SIZE) {
    data.set_leaf(node_index, leaves);
  } else {
    internal::SpheresSplit ss= divide_spheres(spheres, leaves);
    unsigned int nc= data.add_children(node_index, ss.size());
    for (unsigned int i=0; i< ss.size(); ++i) {
      setup(spheres, nc+i, ss[i], data);
    }
  }
}

/* create a tree in a vector where the stored data is
   - indexes of children
   - bounding sphere (in the rigid body internal frame)

   a node always has children, even it is a leaf (if it is a leaf, the child
   is itself). Encode being a leaf by having a negative last index, that being
   the index into the array of particles.
 */
void RigidClosePairsFinder::setup(Particle *p) const {
  if (RigidBody::is_instance_of(p)) {
    // build spheres on internal coordinates
    RigidBody d(p);
    algebra::Sphere3Ds spheres(d.get_number_of_members());
    for (unsigned int i=0; i< d.get_number_of_members(); ++i) {
      double r =get_radius(d.get_member(i).get_particle());
      algebra::Vector3D v= d.get_member(i).get_internal_coordinates();
      spheres[i]= algebra::Sphere3D(v, r);
    }
    data_.add_data(p);
    // call internal setup on spheres, 0, all indexes
    internal::SphereIndexes leaves(d.get_number_of_members());
    for (unsigned int i=0; i< leaves.size(); ++i) {
      leaves[i]=i;
    }
    setup(spheres, 0, leaves, data_.get_data(p));
  } else {
    double r=get_radius(p);
    XYZ d(p);
    data_.add_data(p, algebra::Sphere3D(d.get_coordinates(),
                            (r+get_distance()/2.0)*EXPANSION));
  }
  //IMP_LOG_WRITE(VERBOSE, data_.get_data(p).show_tree(IMP_STREAM, p));
}


std::vector<algebra::Sphere3D>
RigidClosePairsFinder::get_tree(Particle *p) const {
  std::vector<algebra::Sphere3D> ret= data_.get_data(p).get_spheres();
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= get_transformed(p, ret[i]);
  }
  return ret;
}

void RigidClosePairsFinder::show_tree(Particle *p, std::ostream &out) const {

  return data_.get_data(p).show_tree(out, p);
}

namespace {
  void check_particles(SingletonContainer *sc, FloatKey rk) {
    IMP_IF_CHECK(CHEAP) {
      for (SingletonContainer::ParticleIterator it= sc->particles_begin();
         it != sc->particles_end(); ++it) {
        if (RigidBody::is_instance_of(*it)
            && !(*it)->has_attribute(rk)) {
          IMP_WARN("Particle " << (*it)->get_name() << " is a rigid body "
                   << "but does not have a radius. "
                   << "Collision detection is unlikely to work.");
        }
      }
    }
  }
}


void RigidClosePairsFinder
::add_close_pairs(SingletonContainer *ca,
                  SingletonContainer *cb,
                  ListPairContainer *out) const {
  IMP_LOG(TERSE, "Rigid add_close_pairs called with "
          << ca->get_number_of_particles() << " and "
          << cb->get_number_of_particles() << std::endl);
  EditGuard<ListPairContainer> e(out);
  check_particles(ca, get_radius_key());
  check_particles(cb, get_radius_key());
  cpf_->add_close_pairs(ca,cb, cpfout_);
  for (ListPairContainer::ParticlePairConstIterator
         it= cpfout_->particle_pairs_begin();
       it != cpfout_->particle_pairs_end(); ++it) {
    add_close_pairs(it->first, it->second, out);
  }
}

void RigidClosePairsFinder
::add_close_pairs(SingletonContainer *c,
                  ListPairContainer *out) const {
  IMP_LOG(TERSE, "Adding close pairs from "
          << c->get_number_of_particles() << " particles." << std::endl);
  EditGuard<ListPairContainer> e(out);
  check_particles(c, get_radius_key());
  cpf_->add_close_pairs(c, cpfout_);
  for (ListPairContainer::ParticlePairConstIterator
         it= cpfout_->particle_pairs_begin();
       it != cpfout_->particle_pairs_end(); ++it) {
     add_close_pairs(it->first, it->second, out);
  }
}

inline void
RigidClosePairsFinder::process_one(Particle *a, Particle *b,
                                   ListPairContainer *out,
                                   const internal::RigidBodyParticleData &da,
                                   const internal::RigidBodyParticleData &db,
                                   unsigned int ci,
                                   unsigned int cj,
                        std::vector<std::pair<int, int> > &stack) const {
  if (da.get_is_leaf(ci) && db.get_is_leaf(cj)) {
    for (unsigned int k=0; k< da.get_number_of_particles(ci); ++k) {
      Particle *pk=da.get_particle(ci, k, a);
      algebra::Sphere3D sk(XYZ(pk).get_coordinates(),
                             get_radius(pk)+get_distance()*.5);
      for (unsigned int l=0; l< db.get_number_of_particles(cj); ++l) {
        Particle *pl=db.get_particle(cj, l, b);
        algebra::Sphere3D sl(XYZ(pl).get_coordinates(),
                             get_radius(pl)+get_distance()*.5);
        /*IMP_LOG(VERBOSE, "Trying member particles " << pk->get_name()
          << " and " << pl->get_name() << std::endl);*/
        if (interiors_intersect(sk, sl)) {
          out->add_particle_pair(ParticlePair(pk, pl));
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


void RigidClosePairsFinder::add_close_pairs(Particle *a, Particle *b,
                                            ListPairContainer *out) const {
  if (!data_.has_data(a)) {
    setup(a);
  }
  if (!data_.has_data(b)) {
    setup(b);
  }

  EditGuard<ListPairContainer> e(out);
  /*IMP_LOG(VERBOSE, "Testing " << a->get_name() << " and " << b->get_name()
    << " for addition to list" << std::endl);*/
  const internal::RigidBodyParticleData &da= data_.get_data(a);
  const internal::RigidBodyParticleData &db= data_.get_data(b);
  std::vector<std::pair<int, int> > stack;
  process_one(a,b,out, da, db, 0,0,stack);
  while (!stack.empty()) {
    std::pair<int, int> cur= stack.back();
    stack.pop_back();
    /*IMP_LOG(VERBOSE, "Processing pair " << cur.first << " "
      << cur.second << std::endl);*/
    for (unsigned int i=0; i< da.get_number_of_children(cur.first);
         ++i) {
      int ci=da.get_child(cur.first, i);
      algebra::Sphere3D si = get_transformed(a, da.get_sphere(ci));
      for (unsigned int j=0;
           j< db.get_number_of_children(cur.second); ++j) {
        int cj=db.get_child(cur.second, j);
        algebra::Sphere3D sj = get_transformed(b, db.get_sphere(cj));
        if (interiors_intersect(si, sj)) {
          process_one(a,b,out, da, db, ci, cj, stack);
        } else {
          /*IMP_LOG(VERBOSE, "Rejected " << ci << " " << cj << ": "
            << si << " | " << sj << std::endl);*/
        }
      }
    }
  };
}

IMPCORE_END_NAMESPACE
