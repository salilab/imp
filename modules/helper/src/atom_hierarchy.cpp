/**
 *  \file covers.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/helper/atom_hierarchy.h"
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Fragment.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/bond_decorators.h>
#include <IMP/atom/estimates.h>
#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/Harmonic.h>
#include <IMP/core/IncrementalBallMover.h>
#include <IMP/core/ListSingletonContainer.h>
#include <IMP/core/model_io.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/Mover.h>
#include <IMP/core/internal/Grid3D.h>
#include <IMP/display/LogOptimizerState.h>
#include <IMP/display/ChimeraWriter.h>
#include <IMP/display/xyzr_geometry.h>
#include <IMP/em/FitRestraint.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/SampledDensityMap.h>

IMPHELPER_BEGIN_NAMESPACE



namespace {
  std::pair<int, double> compute_n(double V, double r, double f) {
    double n=.5*(3*V+2*PI*cube(r*f)-6*PI*cube(r)*square(f))
      /((-3*square(f)+cube(f)+2)*cube(r)*PI);
    int in= static_cast<int>(std::ceil(n));
    double rr= std::pow(V/(.666*(2*in-3*square(f)*n+cube(f)*n
                                 +3*square(f)-cube(f))*PI), .333333);
    return std::make_pair(in, rr);
  }
}

Restraint* create_protein(Particle *p,
                          double resolution,
                          int number_of_residues,
                          int first_residue_index,
                          double volume,
                          double spring_strength) {
  double mass= atom::mass_in_kDa_from_number_of_residues(number_of_residues);
  if (volume < 0) {
    volume= atom::volume_from_mass_in_kDa(mass);
  }
  // assume a 20% overlap in the beads to make the protein not too bumpy
  double overlap_frac=.2;
  std::pair<int, double> nr= compute_n(volume, resolution, overlap_frac);
  atom::Hierarchy pd
    =atom::Hierarchy::setup_particle(p,
                                     atom::Hierarchy::PROTEIN);
  Particles ps;
  for (int i=0; i< nr.first; ++i) {
    Particle *pc= new Particle(p->get_model());
    atom::Hierarchy pcd
      =atom::Hierarchy::setup_particle(pc,
                                       atom::Hierarchy::FRAGMENT);
    pd.add_child(pcd);
    core::XYZR xyzd=core::XYZR::setup_particle(pc);
    xyzd.set_radius(nr.second);
    xyzd.set_coordinates_are_optimized(true);
    ps.push_back(pc);
    atom::Domain::setup_particle(pc, i*(number_of_residues/nr.first)
                                 + first_residue_index,
                                 (i+1)*(number_of_residues/nr.first)
                                 + first_residue_index);
    atom::Mass::setup_particle(pc, mass/nr.first);
  }
  IMP_NEW(core::Harmonic, h, ((1-overlap_frac)*2*nr.second, spring_strength));
  IMP_NEW(core::DistancePairScore, dps, (h));
  core::ConnectivityRestraint* cr= new core::ConnectivityRestraint(dps);
  cr->set_particles(ps);
  return cr;
}


namespace {
  typedef std::vector<algebra::Sphere3D> Spheres;
  typedef std::vector<algebra::Vector3D> Vectors;
  typedef std::vector<unsigned int> Indexes;


  double cell_size(double resolution) {
    return resolution/6;
  }

  algebra::Sphere3D get_sphere(Particle *p) {
    if (core::XYZR::particle_is_instance(p)) {
      return core::XYZR(p).get_sphere();
    } else {
      return algebra::Sphere3D(core::XYZ(p).get_coordinates(),
                               1);
    }
  }


  void teleport(Spheres &centers, double resolution,
                const Indexes &offsets) {
    static int dir=0;
    ++dir;
    double minr= std::numeric_limits<double>::max();
    unsigned int maxo=0;
    int minc=-1;
    int maxc=-1;
    for (unsigned int i=0; i< centers.size(); ++i) {
      if (centers[i].get_radius() < minr) {
        minr= centers[i].get_radius();
        minc= i;
      }
      if (offsets[i] > maxo) {
        maxo= offsets[i];
        maxc=i;
      }
    }
    IMP_LOG(VERBOSE, "Teleporting " << centers[minc] << " to ");
    // move minr to near maxr
    algebra::Vector3D offset= algebra::basis_vector<3>(dir%3)
      * cell_size(resolution);
    centers[minc]=algebra::Sphere3D(centers[maxc].get_center()
                                    +offset,
                                    centers[maxc].get_radius()/2.0);
    centers[maxc]=algebra::Sphere3D(centers[maxc].get_center()
                                    -offset,
                                    centers[maxc].get_radius()/2.0);
    IMP_LOG(VERBOSE, centers[minc] << std::endl);
  }

  void rasterize_io(atom::Hierarchy in, double res,
                    Vectors &inside,
                    Vectors &outside) {

    algebra::BoundingBox3D bb= get_bounding_box(in);
    bb+= res;
    typedef IMP::core::internal::Grid3D<bool> Grid;
    Grid grid(cell_size(res), bb.get_corner(0), bb.get_corner(1),
              false);
    Particles leaves= get_leaves(in);
    algebra::Vector3D ones(1,1,1);
    for (unsigned int i=0; i< leaves.size(); ++i) {
      algebra::Sphere3D s= get_sphere(leaves[i]);
      //s= algebra::Sphere3D(s.get_center(), s.get_radius());
      algebra::Vector3D lb= s.get_center()-s.get_radius()*ones;
      algebra::Vector3D ub= s.get_center()+s.get_radius()*ones;
      Grid::VirtualIndex lbi= grid.get_virtual_index(lb);
      Grid::VirtualIndex ubt=grid.get_virtual_index(ub);
      Grid::VirtualIndex ubi(ubt[0]+1, ubt[1]+1, ubt[2]+1);
      for (Grid::IndexIterator it= grid.indexes_begin(lbi, ubi);
           it != grid.indexes_end(lbi, ubi); ++it) {
        if (!grid.get_voxel(*it)) {
          algebra::Vector3D center= grid.get_center(*it);
          if (s.get_contains(center)) {
            grid.get_voxel(*it)=true;
          }
        }
      }
      grid.get_voxel(grid.get_index(s.get_center()))=true;
    }
    for (Grid::IndexIterator it= grid.all_indexes_begin();
         it != grid.all_indexes_end(); ++it) {
      if (grid.get_voxel(*it)) {
        inside.push_back(grid.get_center(*it));
      } else {
        outside.push_back(grid.get_center(*it));
      }
    }
  }

  void sort_outside(const Vectors &outside,
                    const Spheres &centers,
                    std::vector< std::vector<float> > &dists) {
    for (unsigned int i=0; i< centers.size(); ++i) {
      dists[i].resize(outside.size());
      for (unsigned int j=0; j< outside.size(); ++j) {
        dists[i][j]= distance(outside[j], centers[i].get_center());
      }
      std::sort(dists[i].begin(), dists[i].end());
      /*IMP_LOG(VERBOSE, "Sphere " << i < <" has lb of " << dists[i].front()
        << " and ub of " << dists[i].back() << std::endl);*/
    }
  }

  int assign(const std::vector< std::vector<float> > &dists,
             const Spheres &centers,
             const Indexes &offsets,
             const algebra::Vector3D &v,
             std::size_t &best_score) {
    best_score=dists[0].size();
    int best_center=0;
    int cur_best=200000;
    for (unsigned int j=0; j< centers.size(); ++j) {
      float dist = distance(v, centers[j].get_center());
      std::vector<float>::const_iterator it
        = std::upper_bound(dists[j].begin(),
                           dists[j].begin()
                           +std::min(best_score,
                                     dists[j].size()),
                           dist);
      if (it == dists[j].begin()
          +std::min(best_score,
                    dists[j].size())) continue;
      /*IMP_LOG(VERBOSE, "Point " << i << " center " << j
        << " dist " << dist << " bound " << *it << std::endl);*/
      unsigned int score;
      /*if (it -dists[j].begin() < offsets[j]) {
        score=0;
        } else {*/
      score= std::distance(dists[j].begin(), it);
      unsigned int nscore=score;
      if (nscore < offsets[j]) nscore=0;
      else nscore= nscore-offsets[j];
      //}
      if (static_cast<int>(nscore) < cur_best) {
        best_score=score;
        best_center=j;
        cur_best= nscore;
      }
    }
    return best_center;
  }


  void compute_assignments(const Vectors &inside,
                           const std::vector< std::vector<float> > &dists,
                           const Spheres &centers,
                           Indexes &offsets,
                           Indexes &assignments) {
    for (unsigned int i=0; i< inside.size(); ++i) {
      std::size_t score;
      assignments[i]= assign(dists, centers, offsets, inside[i],
                             score);
      offsets[assignments[i]]= std::max(offsets[assignments[i]],
                                        static_cast<unsigned int>(score));
    }
  }

  void recompute_centers(const Vectors &inside, const Indexes &assignments,
                         double resolution,
                         Spheres &centers) {
    boost::uniform_int<int> r(0, inside.size()-1);
    for (unsigned int i=0; i< centers.size(); ++i) {
      algebra::Sphere3Ds cur;
      for (unsigned int j=0; j< inside.size(); ++j) {
        if (assignments[j]==i) {
          cur.push_back(algebra::Sphere3D(inside[j], 0));
        }
      }
      if (cur.empty()) {
        int index= r(random_number_generator);
        centers[i]= algebra::Sphere3D(inside[index],
                                      0);
      } else if (cur.size() ==1) {
        centers[i]= algebra::Sphere3D(cur[0].get_center(),
                                      cell_size(resolution));
      } else {
        centers[i]= enclosing_sphere(cur);
      }
    }
  }

  void write_spheres(Spheres &centers,
                     unsigned int index) {
    std::ostringstream oss;
    oss << "Spheres." << index << ".bild";
    std::ofstream out(oss.str().c_str());
    for (unsigned int i=0; i< centers.size(); ++i) {
      out << ".sphere " << spaces_io(centers[i].get_center())
          << " " << centers[i].get_radius() << std::endl;
    }
  }

  void write_outside(const Vectors &vs) {
    std::ostringstream oss;
    oss << "Outside"
        << ".bild";
    std::ofstream out(oss.str().c_str());
    out << ".color 10\n";
    for (unsigned int i=0; i< vs.size(); ++i) {
      out << ".sphere " << spaces_io(vs[i])
          << " " << .5 << std::endl;
    }
  }

  void write_assignments(const Vectors &inside,
                         const Indexes &assignments,
                         unsigned int index) {
    std::ostringstream oss;
    oss << "Assignments." << index << ".bild";
    std::ofstream out(oss.str().c_str());
    for (unsigned int i=0; i< assignments.size(); ++i) {
      out << ".color " << assignments[i]*5 << std::endl;
      out << ".sphere " << spaces_io(inside[i])
          << " " << .5 << std::endl;
    }
  }

  double compute_score(const Spheres &centers,
                       const std::vector<std::vector<float> > &dists) {
    double sum=0;
    for (unsigned int i=0; i< centers.size(); ++i) {
      double score= std::upper_bound(dists[i].begin(),
                                     dists[i].end(),
                                     centers[i].get_radius())-dists[i].begin();
      sum+= score;
    }
    return sum;
  }


  struct CompareFirst {
    template <class T>
    bool operator()(const T &a, const T &b) {
      return a.first < b.first;
    }
  };

  void sort_inside(const Spheres &centers,
                   Vectors& inside) {
    std::vector<std::pair<float, int> > dists(inside.size());
    for (unsigned int i=0; i< inside.size(); ++i) {
      float d=1000000;
      for (unsigned int j=0; j< centers.size(); ++j) {
        float cd= distance(centers[j].get_center(), inside[i]);
        d= std::min(cd, d);
      }
      dists[i]= std::pair<float,int>(d, i);
    }
    std::sort(dists.begin(), dists.end(), CompareFirst());
    Vectors t;
    std::swap(t,inside);
    inside.resize(t.size());
    for (unsigned int i=0; i< t.size(); ++i) {
      inside[i]= t[dists[i].second];
    }
  }

  void partition_particles(const algebra::Sphere3Ds &centers,
                           const Particles &ps,
                           std::vector<Particles> &out) {
    IMP_LOG(VERBOSE, "Partitioning " << ps.size() << " particles"<<std::endl);
    out.resize(centers.size());
    for (unsigned int i=0; i< ps.size(); ++i) {
      algebra::Vector3D v= core::XYZ(ps[i]).get_coordinates();
      double close_distance=10000;
      int close_center=-1;
      for (unsigned int j=0; j< centers.size(); ++j) {
        double d= algebra::distance(v, centers[j].get_center())
          -centers[j].get_radius();
        if (d < close_distance) {
          close_distance=d;
          close_center=j;
        }
      }
      IMP_assert(close_center >=0, "Nothing close found");
      out[close_center].push_back(ps[i]);
    }
  }




  /*************************************************************

   Below this is for the _2 version

  *************************************************************/

  void setup_fragments(const atom::Fragments& fragments,
                       const atom::Hierarchies &leaves) {
    std::vector<Ints> residue_indexes(fragments.size());
    std::vector<double> masses(fragments.size(), 0);

    for (unsigned int i=0; i< leaves.size(); ++i) {
      algebra::Vector3D v= core::XYZ(leaves[i]).get_coordinates();
      double md=std::numeric_limits<double>::max();
      int mj=-1;
      for (unsigned int j=0; j< fragments.size(); ++j) {
        algebra::Sphere3D s= core::XYZR(fragments[i]).get_sphere();
        double d= distance(s.get_center(), v)-s.get_radius();
        if (d < md) {
          md=d;
          mj=j;
        }
      }
      masses[mj]+= atom::Mass(leaves[i]).get_mass();
      if (atom::Atom::particle_is_instance(leaves[i])) {
        residue_indexes[mj]
          .push_back(get_residue(atom::Atom(leaves[i])).get_index());
      } else if (atom::Residue::particle_is_instance(leaves[i])) {
        residue_indexes[mj]
          .push_back(atom::Residue(leaves[i]).get_index());
      } else if (atom::Fragment::particle_is_instance(leaves[i])) {
        Ints cis= atom::Fragment(leaves[i]).get_residue_indexes();
        residue_indexes[mj].insert(residue_indexes[mj].end(),
                                   cis.begin(), cis.end());
      }
    }

    for (unsigned int i=0; i< residue_indexes.size(); ++i) {
      std::sort(residue_indexes[i].begin(), residue_indexes[i].end());
      residue_indexes[i].erase(std::unique(residue_indexes[i].begin(),
                                           residue_indexes[i].end()),
                            residue_indexes[i].end());
      fragments[i].set_residue_indexes(residue_indexes[i]);
      atom::Mass(fragments[i]).set_mass(masses[i]);
    }
  }





  class IncrementalRadiusMassMover :public core::Mover
  {
  public:
    IncrementalRadiusMassMover(SingletonContainer *sc,
                               unsigned int n,
                               Float delta_radius);
    IMP_MOVER(IncrementalRadiusMassMover, get_module_version_info())
    private:
    Pointer<SingletonContainer> sc_;
    unsigned int n_;
    Float radius_delta_;
    ParticlesTemp moved_;
  };



  IncrementalRadiusMassMover
  ::IncrementalRadiusMassMover(SingletonContainer *sc,
                               unsigned int n,
                               Float radius): sc_(sc),
                                              n_(n),
                                              radius_delta_(radius),
                                              moved_(n_)
  {
  }

  void IncrementalRadiusMassMover::propose_move(Float size) {
    // damnit, why didn't these functions make it into the standard
    /*std::random_sample(sc_->particles_begin(), sc_->particles_end(),
      moved_.begin(), moved_.end());*/
    ParticlesTemp pts(sc_->get_particles());
    std::random_shuffle(pts.begin(), pts.end());
    for (unsigned int i=0; i< n_; ++i) {
      moved_[i]=pts[i];
      core::XYZR d(moved_[i]);
      atom::Mass m(moved_[i]);
      double ro= d.get_radius();
      d.set_radius(algebra::random_vector_in_sphere<1>
                   (algebra::VectorD<1>(d.get_radius()),
                    radius_delta_)[0]);
      double mn= m.get_mass()/std::pow(ro,3) * std::pow(d.get_radius(),3);
      m.set_mass(mn);
    }
  }


  void IncrementalRadiusMassMover::accept_move() {
  }

  void IncrementalRadiusMassMover::reject_move() {
    for (unsigned int i=0; i< n_; ++i) {
      core::XYZR odr(moved_[i]->get_prechange_particle());
      core::XYZR cdr(moved_[i]);
      atom::Mass odm(moved_[i]->get_prechange_particle());
      atom::Mass cdm(moved_[i]);
      cdr.set_radius(odr.get_radius());
      cdm.set_mass(odm.get_mass());
    }
  }


  void IncrementalRadiusMassMover::show(std::ostream &out) const {
    out << "IncrementalRadiusMassMover" << std::endl;
  }

}

/** Simplify a molecular hierarchy to be represented by fewer
    balls.
    given current center locations, perform iterations of:
    - assign each point in order of distance to the nearest center to the
    center where addition minimizes outside volume (nkE)
    - recenter centers (nk)
    - if no improvement, move a center to split the largest sphere

    To measure error:
    - sort all outside points based on all their distances to each center
    (knlogn)
    - see how many new ones are needed, E=(1)

    Then:
    - take each amino acid, figure out which sphere it is closest to
    add it as a child of that sphere with no coordinates
*/
atom::Hierarchy create_simplified(atom::Hierarchy in,
                               double resolution) {
  // compute outside points
  Vectors outside;
  Vectors inside;
  IMP_LOG(VERBOSE, "Rasterizing protein " << std::flush);
  rasterize_io(in, resolution, inside, outside);
  //write_outside(outside);
  IMP_LOG(VERBOSE, inside.size() << " inside and " << outside.size()
          << " outside " << std::endl);
  std::random_shuffle(inside.begin(), inside.end());
  // compute initial number of centers
  int num_centers=inside.size()/40;
  IMP_LOG(VERBOSE, num_centers << " centers" << std::endl);

  // produce random initial centers
  Spheres centers(num_centers);
  boost::uniform_int<int> r(0, inside.size()-1);
  for (unsigned int i=0; i< centers.size(); ++i) {
    int index= r(random_number_generator);
    centers[i]= algebra::Sphere3D(inside[index], 10000);
  }
  //write_spheres(centers, 0);
  Indexes assignments(inside.size());
  double old_score= std::numeric_limits<double>::max();
  std::vector< std::vector<float> > dists(centers.size());
  unsigned int teleports=0;
  for (unsigned int i=0; i<100; ++i) {
    // generate sorted lists of distance

    IMP_LOG(VERBOSE, "Sorting outside " << std::endl);
    sort_outside(outside, centers, dists);
    Indexes offsets(centers.size());
    double new_score= compute_score(centers, dists);
    IMP_LOG(VERBOSE, "Score is " << new_score << std::endl);

    if (new_score==0) break;
    if (old_score <= new_score) {
      if (teleports ==10) break;
      IMP_LOG(VERBOSE, "Computing assignments " << std::endl);
      compute_assignments(inside, dists, centers, offsets,  assignments);
      IMP_LOG(VERBOSE, "teleporting " << std::endl);
      teleport(centers, resolution, offsets);
      ++teleports;
      sort_outside(outside, centers, dists);
    }
    old_score=new_score;
    IMP_LOG(VERBOSE, "Sorting into addition order " << std::endl);
    sort_inside(centers, inside);
    IMP_LOG(VERBOSE, "Computing assignments " << std::endl);
    compute_assignments(inside, dists, centers, offsets,  assignments);
    //write_assignments(inside, assignments, i);
    IMP_LOG(VERBOSE, "Recomputing centers " << std::endl);
    recompute_centers(inside, assignments, resolution, centers);
    //write_spheres(centers, i+1);
  }

  IMP_NEW(Particle, root, (in.get_particle()->get_model()));
  atom::Hierarchy h= atom::Hierarchy::setup_particle(root, in.get_type());
  for (unsigned int i=0; i< centers.size(); ++i) {
    IMP_NEW(Particle, c, (in.get_particle()->get_model()));
    atom::Hierarchy hc
      = atom::Hierarchy::setup_particle(c, atom::Hierarchy::FRAGMENT);
    h.add_child(hc);
    core::XYZR::setup_particle(c, centers[i]);
  }


  // copy residue information
  std::vector<Particles> partitioned;
  partition_particles(centers, get_leaves(in), partitioned);
  std::vector<Ints> residues(centers.size());
  for (unsigned int i=0; i < partitioned.size(); ++i) {
    for (unsigned int j=0; j < partitioned[i].size(); ++j) {
      if (atom::Atom::particle_is_instance(partitioned[i][j])) {
        atom::Atom at(partitioned[i][j]);
        if (at.get_atom_type() == atom::AT_CA) {
          residues[i].push_back(atom::Residue(at.get_parent()
                                              .get_particle()).get_index());
        }
      } else if (atom::Residue::particle_is_instance(partitioned[i][j])) {
        residues[i].push_back(atom::Residue(partitioned[i][j]).get_index());
      } else if (atom::Fragment::particle_is_instance(partitioned[i][j])) {
        atom::Fragment f(partitioned[i][j]);
        residues[i].insert(residues[i].end(),
                           f.residue_indexes_begin(),
                           f.residue_indexes_end());
      } else if (atom::Domain::particle_is_instance(partitioned[i][j])) {
        atom::Domain d(partitioned[i][j]);
        for (int k=d.get_begin_index(); k != d.get_end_index(); ++k) {
          residues[i].push_back(k);
        }
      }
    }
  }
  for (unsigned int i=0; i< centers.size(); ++i) {
    atom::Fragment f
      = atom::Fragment::setup_particle(h.get_child(i).get_particle());
    if (!residues[i].empty()) {
      f.set_residue_indexes(residues[i]);
    }
  }
  IMP_assert(h.get_is_valid(true), "Produced invalid hierarchy");
  return h;
}


/*
  r= resolution/2.0
  n*4/3 pi *r^3=2*v
  n= 3/2*v/(pi r^3)
*/
atom::Hierarchy create_simplified_2(atom::Hierarchy in,
                                    double resolution) {
  IMP_check(in.get_type() == atom::Hierarchy::PROTEIN,
            "Can only simplify proteins at the moment.",
            ValueException);
  atom::Hierarchies leaves= atom::get_leaves(in);

  // rasterize protein to grid with size resolution/2.0
  IMP_NEW(em::SampledDensityMap, sdm,
          (em::IMPParticlesAccessPoint(leaves,
                          core::XYZR::get_default_radius_key(),
                          atom::Mass::get_mass_key()),
           resolution, resolution/2.0));
  {
    em::MRCReaderWriter mrc; // for some reason it wants a non-const ref
    em::write_map(sdm, "map.mrc", mrc);
  }

  // add up mass of leaves
  double total_mass=0;
  for (unsigned int i=0; i< leaves.size(); ++i) {
    total_mass+= atom::Mass(leaves[i]).get_mass();
  }

  // estimate volume using grid
  int vin=0;
  for (unsigned int i=0; i< sdm->get_number_of_voxels(); ++i) {
    if (sdm->get_value(i) > 8.17e-06) { // WHAT VALUE HERE?
      ++vin;
    }
  }
  double v=vin*std::pow(resolution, 3)/8.0;
  IMP_LOG(TERSE, "Estimated volume is " << v << std::endl);


  // compute parameters
  unsigned int n=1;//std::ceil(12*v/(PI *std::pow(resolution, 3)));
  algebra::BoundingBox3D bb=get_bounding_box(in);
  double r0=std::pow((3.0/4)*v/n, .33333);
  double m0= 2.0*total_mass/n;
  IMP_LOG(TERSE, "Using " << n << " spheres." << std::endl);

  // create new model and particles to optimize
  // TODO use qmm or something to pick starting points
  IMP_NEW(Model, m, ());
  m->set_is_incremental(true);
  core::XYZRs xyzrs(n);
  //FloatKey weight("sillykey");
  for (unsigned int i=0; i< n; ++i) {
    xyzrs.set(i,
              core::XYZR::setup_particle(new Particle(m),
                    algebra::Sphere3D(algebra::random_vector_in_box(bb),
                                      r0)));
    xyzrs[i].set_coordinates_are_optimized(true);
    atom::Mass::setup_particle(xyzrs[i], m0);
    //xyzrs[i].get_particle()->add_attribute(weight, 1, false);
  }
  IMP_NEW(core::ListSingletonContainer, lsc, (xyzrs));

  // create MC
  IMP_NEW(core::MonteCarlo, mc, ());
  {
    IMP_NEW(display::LogOptimizerState, los,
            (new display::ChimeraWriter(), "frame.%04d.py"));
    mc->add_optimizer_state(los);
    IMP_NEW(display::XYZRsGeometry, xyzrg, (lsc));
    los->add_geometry(xyzrg);
    los->write("initial.py");
    los->set_skip_steps(0);
    IMP_NEW(core::DumpModelOnFailure, dmf, (m, "error.imp"));
    add_failure_handler(dmf);
    IMP_NEW(display::DisplayModelOnFailure, lof,
            (los, "error.py"));
    add_failure_handler(lof);
  }
  mc->set_model(m);
  mc->set_local_optimizer(new core::ConjugateGradients());
  mc->set_local_steps(10);
  mc->add_mover(new core::IncrementalBallMover(lsc, 1,
                                               resolution/2.0));
  mc->add_mover(new IncrementalRadiusMassMover(lsc, 1, resolution/2.0));
  // set up fit restraint
  IMP_NEW(em::FitRestraint, ccr, (xyzrs, sdm,
                                  core::XYZR::get_default_radius_key(),
                                  atom::Mass::get_mass_key()));
  m->add_restraint(ccr);
  // run MC

  mc->optimize(1000);
  /*remove_failure_handler(dmf);
    remove_failure_handler(lof);*/

  // copy centers back
  atom::Fragments frags(n);
  for (unsigned int i=0; i< n; ++i) {
    frags.set(i, atom::Fragment::setup_particle(new Particle(in.get_model())));
    core::XYZR::setup_particle(frags[i], xyzrs[i].get_sphere());
  }
  // X assign residues and masses
  setup_fragments(frags, leaves);
  // create hierarchy
  atom::Hierarchy ret
    = atom::Hierarchy::setup_particle(new Particle(in.get_model()),
                                      atom::Hierarchy::PROTEIN);
  for (unsigned int i=0; i< frags.size(); ++i) {
    ret.add_child(frags[i]);
  }
  return ret;
}



Particles
get_simplified_representation(atom::Hierarchy h) {
  Particles ret;
  std::vector<atom::Hierarchy> front;
  front.push_back(h);
  do {
    atom::Hierarchy h= front.back();
    front.pop_back();
    //IMP_LOG(VERBOSE, "Trying " << h << std::endl);
    if (core::XYZ::particle_is_instance(h.get_particle())) {
      ret.push_back(h.get_particle());
    } else {
      front.insert(front.end(),
                   h.children_begin(),
                   h.children_end());
    }
  } while (!front.empty());
  return ret;
}




namespace {

  atom::Hierarchy clone_internal(atom::Hierarchy d,
                                 std::map<Particle*,
                                 Particle*> &map) {
    Particle *p= new Particle(d.get_model());
    map[d.get_particle()]=p;
    atom::Hierarchy nd;
    if (atom::Atom::particle_is_instance(d.get_particle())) {
      nd= atom::Atom::setup_particle(p, atom::Atom(d.get_particle()));
    } else if (atom::Residue::particle_is_instance(d.get_particle())) {
      nd= atom::Residue::setup_particle(p, atom::Residue(d.get_particle()));
    } else if (atom::Domain::particle_is_instance(d.get_particle())) {
      nd= atom::Domain::setup_particle(p, atom::Domain(d.get_particle()));
    } else if (atom::Chain::particle_is_instance(d.get_particle())) {
      nd= atom::Chain::setup_particle(p, atom::Chain(d.get_particle()));
    } else if (atom::Fragment::particle_is_instance(d.get_particle())) {
      nd= atom::Fragment::setup_particle(p, atom::Fragment(d.get_particle()));
    } else {
      nd=atom::Hierarchy::setup_particle(p, d.get_type());
    }
    using core::XYZ;
    using core::XYZR;
    if (XYZR::particle_is_instance(d.get_particle())){
      XYZR::setup_particle(p,
                           algebra::Sphere3D(XYZ(d.get_particle())
                                             .get_coordinates(),
                                             XYZR(d.get_particle())
                                             .get_radius()));
    } else if (XYZ::particle_is_instance(d.get_particle())) {
      XYZ::setup_particle(p,
                          XYZ(d.get_particle()).get_coordinates());
    }
    p->set_name(d.get_particle()->get_name());
    for (unsigned int i=0 ;i< d.get_number_of_children(); ++i) {
      atom::Hierarchy nc= clone_internal(d.get_child(i), map);
      nd.add_child(nc);
    }
    return nd;
  }
}


atom::Hierarchy clone(atom::Hierarchy d) {
  std::map<Particle*,Particle*> map;
  atom::Hierarchy nh= clone_internal(d, map);
  atom::Bonds bds= get_internal_bonds(d);
  for (unsigned int i=0; i< bds.size(); ++i) {
    atom::Bonded e0= bds[i].get_bonded(0);
    atom::Bonded e1= bds[i].get_bonded(1);
    Particle *np0= map[e0.get_particle()];
    Particle *np1= map[e1.get_particle()];
    atom::Bonded ne0, ne1;
    if (atom::Bonded::particle_is_instance(np0)) {
      ne0=atom::Bonded(np0);
    } else {
      ne0=atom::Bonded::setup_particle(np0);
    }
    if (atom::Bonded::particle_is_instance(np1)) {
      ne1=atom::Bonded(np1);
    } else {
      ne1=atom::Bonded::setup_particle(np1);
    }
    atom::copy_bond(ne0, ne1, bds[i]);
  }
  return nh;
}




algebra::BoundingBox3D get_bounding_box(const atom::Hierarchy &h,
                                        FloatKey r) {
  Particles rep= get_simplified_representation(h);
  algebra::BoundingBox3D bb;
  for (unsigned int i=0; i< rep.size(); ++i) {
    core::XYZR xyzr= core::XYZR::decorate_particle(rep[i], r);
    if (xyzr) {
      bb+= algebra::get_bounding_box(xyzr.get_sphere());
    } else if (core::XYZ::particle_is_instance(rep[i])) {
      bb+= algebra::BoundingBox3D(core::XYZ(rep[i]).get_coordinates());
    }
  }
  IMP_LOG(VERBOSE, "Bounding box is " << bb << std::endl);
  return bb;
}

algebra::Sphere3D bounding_sphere(const atom::Hierarchy &h,
                                  FloatKey r) {
  Particles rep= get_simplified_representation(h);
  algebra::Sphere3Ds ss;
  for (unsigned int i=0; i< rep.size(); ++i) {
    core::XYZR xyzr= core::XYZR::decorate_particle(rep[i], r);
    if (xyzr) {
      ss.push_back(xyzr.get_sphere());
    } else if (core::XYZ::particle_is_instance(rep[i])) {
      ss.push_back(algebra::Sphere3D(core::XYZ(rep[i]).get_coordinates(),
                                     0));
    }
  }
  return algebra::enclosing_sphere(ss);;
}

IMPHELPER_END_NAMESPACE
