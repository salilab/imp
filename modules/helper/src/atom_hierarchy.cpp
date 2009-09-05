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
#include <IMP/display/BoxGeometry.h>
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
  double mass= atom::mass_from_number_of_residues(number_of_residues)/1000;
  if (volume < 0) {
    volume= atom::volume_from_mass(mass*1000);
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


struct True {
  template <class T>
  bool operator()(const T &t) const{ return true;}
};

void destroy(atom::Hierarchy d) {
  atom::Hierarchies all;
  core::gather(d, True(), std::back_inserter(all));
  for (unsigned int i=0; i< all.size(); ++i) {
    if (atom::Bonded::particle_is_instance(all[i])) {
      atom::Bonded b(all[i]);
      while (b.get_number_of_bonds() > 0) {
        atom::unbond(b.get_bond(b.get_number_of_bonds()-1));
      }
    }
    while (all[i].get_number_of_children() > 0) {
      all[i].remove_child(all[i].get_child(all[i].get_number_of_children()-1));
    }
  }
  for (unsigned int i=0; i< all.size(); ++i) {
    all[i].get_particle()->get_model()->remove_particle(all[i]);
  }
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
  return algebra::enclosing_sphere(ss);
}

IMPHELPER_END_NAMESPACE
