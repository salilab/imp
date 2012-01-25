/**
 *  \file spb_restraint.h
 *  \brief SPB Restraints
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_RESTRAINT_H
#define IMPMEMBRANE_SPB_RESTRAINT_H
#include "membrane_config.h"
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT Pointer<core::DistancePairScore>
 get_pair_score(FloatRange dist, double kappa);

IMPMEMBRANEEXPORT Pointer<core::SphereDistancePairScore>
 get_sphere_pair_score(FloatRange dist, double kappa);

IMPMEMBRANEEXPORT Pointer<core::SphereDistancePairScore>
 get_sphere_pair_score(double dist, double kappa);

IMPMEMBRANEEXPORT void add_SPBexcluded_volume
 (Model *m,atom::Hierarchies& hs,bool GFP_exc_volume,double kappa);

IMPMEMBRANEEXPORT void add_internal_restraint(Model *m,std::string name,
atom::Molecule protein_a,atom::Molecule protein_b,double kappa,double dist);

IMPMEMBRANEEXPORT void add_my_connectivity
(Model *m,std::string name,atom::Molecule protein, double kappa);

IMPMEMBRANEEXPORT FloatRange get_range_from_fret_class(std::string r_class);

IMPMEMBRANEEXPORT FloatRange get_range_from_fret_value(double r_value);

IMPMEMBRANEEXPORT Pointer<container::MinimumPairRestraint> do_bipartite_mindist
(Model *m,Particles p1,Particles p2,
 Pointer<core::SphereDistancePairScore> sps,bool filter=true);

IMPMEMBRANEEXPORT Pointer<container::MinimumPairRestraint> do_bipartite_mindist
(Model *m,Particles p1,Particles p2,
 Pointer<core::DistancePairScore> sps,bool filter=true);

IMPMEMBRANEEXPORT Pointer<container::MinimumPairRestraint> do_allpairs_mindist
(Model *m,Particles ps,
 Pointer<core::SphereDistancePairScore> sps,bool filter=true);

IMPMEMBRANEEXPORT void add_layer_restraint(Model *m,
container::ListSingletonContainer *lsc, FloatRange range, double kappa);

IMPMEMBRANEEXPORT Pointer<container::MinimumPairRestraint> fret_restraint
(Model *m,
 const atom::Hierarchy& ha,std::string protein_a,std::string residues_a,
 atom::Hierarchies& hb, std::string protein_b, std::string residues_b,
 double r_value,double kappa,bool use_GFP);

IMPMEMBRANEEXPORT Pointer<membrane::FretrRestraint> NEW_fret_restraint
(Model *m, atom::Hierarchies& hs,
 std::string protein_a, std::string residues_a,
 std::string protein_b, std::string residues_b, double r_value,
 FretParameters Fret, double kappa, bool use_GFP);

IMPMEMBRANEEXPORT Pointer<container::MinimumPairRestraint> y2h_restraint
(Model *m,
 const atom::Hierarchy& ha,std::string protein_a,IntRange residues_a,
 atom::Hierarchies& hb,std::string protein_b,IntRange residues_b,
 double kappa);

IMPMEMBRANEEXPORT Pointer<container::MinimumPairRestraint> y2h_restraint
(Model *m,
 const atom::Hierarchy& ha,std::string protein_a,std::string residues_a,
 atom::Hierarchies& hb, std::string protein_b, std::string residues_b,
 double kappa);

IMPMEMBRANEEXPORT Pointer<container::MinimumPairRestraint> y2h_restraint
(Model *m,
 const atom::Hierarchy& ha,std::string protein_a,IntRange residues_a,
 atom::Hierarchies& hb,std::string protein_b,std::string residues_b,
 double kappa);

IMPMEMBRANEEXPORT Pointer<container::MinimumPairRestraint> y2h_restraint
(Model *m,
 const atom::Hierarchy& ha,std::string protein_a,std::string residues_a,
 atom::Hierarchies& hb,std::string protein_b,IntRange residues_b,
 double kappa);

IMPMEMBRANEEXPORT void add_symmetry_restraint
 (Model *m,atom::Hierarchies& hs,algebra::Transformation3Ds transformations);

IMPMEMBRANEEXPORT void add_link(Model *m,
 const atom::Hierarchy& h, std::string protein_a, std::string residues_a,
 atom::Hierarchies& hs, std::string protein_b, IntRange residues_b,
 double kappa);

IMPMEMBRANEEXPORT void add_link(Model *m,
 const atom::Hierarchy& h, std::string protein_a, std::string residues_a,
 atom::Hierarchies& hs, std::string protein_b, std::string residues_b,
 double kappa);

IMPMEMBRANEEXPORT core::RigidBodies get_rigid_bodies(Particles ps);

IMPMEMBRANEEXPORT void add_tilt(Model *m,
 const atom::Hierarchy& h, std::string name, double tilt, double kappa);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_RESTRAINT_H */
