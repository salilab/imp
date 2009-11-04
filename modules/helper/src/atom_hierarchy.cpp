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
    =atom::Hierarchy::setup_particle(p);
  Particles ps;
  for (int i=0; i< nr.first; ++i) {
    Particle *pc= new Particle(p->get_model());
    atom::Hierarchy pcd
      =atom::Fragment::setup_particle(pc);
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

IMPHELPER_END_NAMESPACE
