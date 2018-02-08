/**
 *  \file IMP/spb/spb_restraint.h
 *  \brief SPB Restraints
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_SPB_RESTRAINT_H
#define IMPSPB_SPB_RESTRAINT_H
#include <IMP.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include <IMP/em2d.h>
#include <IMP/isd.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

IMPSPBEXPORT IMP::Pointer<core::DistancePairScore> get_pair_score(
    FloatRange dist, double kappa);

IMPSPBEXPORT IMP::Pointer<core::SphereDistancePairScore>
get_sphere_pair_score(FloatRange dist, double kappa);

IMPSPBEXPORT IMP::Pointer<core::SphereDistancePairScore>
get_sphere_pair_score(double dist, double kappa);

IMPSPBEXPORT void add_SPBexcluded_volume(Model *m, RestraintSet *allrs,
                                              atom::Hierarchies &hs,
                                              bool GFP_exc_volume,
                                              double kappa);

IMPSPBEXPORT void add_internal_restraint(Model *m, RestraintSet *allrs,
                                              std::string name,
                                              atom::Molecule protein_a,
                                              atom::Molecule protein_b,
                                              double kappa, double dist);

IMPSPBEXPORT void add_my_connectivity(Model *m, RestraintSet *allrs,
                                           std::string name,
                                           atom::Molecule protein,
                                           double kappa);

IMPSPBEXPORT void add_restrain_protein_length(
    RestraintSet *allrs, const atom::Hierarchy &hs,
    std::string protein_a, Particle *dist, double sigma0_dist);

IMPSPBEXPORT void add_restrain_coiledcoil_to_cterm(
    Model *m, RestraintSet *allrs, const atom::Hierarchy &hs,
    std::string protein_a, Particle *dist, double sigma0_dist);

IMPSPBEXPORT IMP::Pointer<container::MinimumPairRestraint>
do_bipartite_mindist(Model *m, Particles p1, Particles p2,
                     IMP::Pointer<core::SphereDistancePairScore> sps,
                     bool filter = true);

IMPSPBEXPORT IMP::Pointer<container::MinimumPairRestraint>
do_bipartite_mindist(Model *m, Particles p1, Particles p2,
                     IMP::Pointer<core::DistancePairScore> sps,
                     bool filter = true);

IMPSPBEXPORT void add_layer_restraint(
    RestraintSet *allrs, container::ListSingletonContainer *lsc,
    FloatRange range, double kappa);

IMPSPBEXPORT void add_bayesian_layer_restraint(
    RestraintSet *allrs, container::ListSingletonContainer *lsc,
    Particle *a, Particle *b);

IMPSPBEXPORT IMP::Pointer<isd::FretRestraint> fret_restraint(
    atom::Hierarchies &hs, std::string protein_a,
    std::string residues_a, std::string protein_b, std::string residues_b,
    double fexp, FretParameters Fret, std::string cell_type, bool use_GFP,
    Particle *Kda, Particle *Ida, Particle *R0, Particle *Sigma0,
    Particle *pBl);

IMPSPBEXPORT IMP::Pointer<isd::FretRestraint> fret_restraint(
    atom::Hierarchies &hs, std::string protein_a,
    std::string residues_a, std::string protein_b, int residues_b, double fexp,
    FretParameters Fret, std::string cell_type, bool use_GFP, Particle *Kda,
    Particle *Ida, Particle *R0, Particle *Sigma0, Particle *pBl);

IMPSPBEXPORT IMP::Pointer<isd::FretRestraint> fret_restraint(
    atom::Hierarchies &hs, std::string protein_a, int residues_a,
    std::string protein_b, std::string residues_b, double fexp,
    FretParameters Fret, std::string cell_type, bool use_GFP, Particle *Kda,
    Particle *Ida, Particle *R0, Particle *Sigma0, Particle *pBl);

IMPSPBEXPORT IMP::Pointer<container::MinimumPairRestraint> y2h_restraint(
    Model *m, const atom::Hierarchy &ha, std::string protein_a,
    IntRange residues_a, atom::Hierarchies &hb, std::string protein_b,
    IntRange residues_b, double kappa);

IMPSPBEXPORT IMP::Pointer<container::MinimumPairRestraint> y2h_restraint(
    Model *m, const atom::Hierarchy &ha, std::string protein_a,
    std::string residues_a, atom::Hierarchies &hb, std::string protein_b,
    std::string residues_b, double kappa);

IMPSPBEXPORT IMP::Pointer<container::MinimumPairRestraint> y2h_restraint(
    Model *m, const atom::Hierarchy &ha, std::string protein_a,
    IntRange residues_a, atom::Hierarchies &hb, std::string protein_b,
    std::string residues_b, double kappa);

IMPSPBEXPORT IMP::Pointer<container::MinimumPairRestraint> y2h_restraint(
    Model *m, const atom::Hierarchy &ha, std::string protein_a,
    std::string residues_a, atom::Hierarchies &hb, std::string protein_b,
    IntRange residues_b, double kappa);

IMPSPBEXPORT void add_symmetry_restraint(
    Model *m, atom::Hierarchies &hs, algebra::Transformation3Ds transformations,
    Particle *SideXY, Particle *SideZ);

IMPSPBEXPORT void add_link(Model *m, RestraintSet *allrs,
                                const atom::Hierarchy &h, std::string protein_a,
                                std::string residues_a, atom::Hierarchies &hs,
                                std::string protein_b, IntRange residues_b,
                                double kappa);

IMPSPBEXPORT void add_link(Model *m, RestraintSet *allrs,
                                const atom::Hierarchy &h, std::string protein_a,
                                std::string residues_a, atom::Hierarchies &hs,
                                std::string protein_b, std::string residues_b,
                                double kappa);

IMPSPBEXPORT std::vector<core::RigidBody> get_rigid_bodies(Particles ps);

IMPSPBEXPORT void add_tilt_restraint(Model *m, RestraintSet *allrs,
                                          Particle *p, FloatRange trange,
                                          double kappa);

IMPSPBEXPORT void add_tilt(Model *m, RestraintSet *allrs,
                                const atom::Hierarchy &h, std::string name,
                                IntRange range, double tilt, double kappa);

IMPSPBEXPORT void add_GFP_restraint(Model *m, RestraintSet *allrs,
                                         const atom::Hierarchy &h,
                                         double kappa);

IMPSPBEXPORT void add_stay_close_restraint(Model *m, RestraintSet *allrs,
                                                const atom::Hierarchy &h,
                                                std::string protein,
                                                double kappa);

IMPSPBEXPORT void add_stay_close_restraint(Model *m, RestraintSet *allrs,
                                                const atom::Hierarchy &h,
                                                std::string protein,
                                                int residue, double kappa);

IMPSPBEXPORT void add_stay_on_plane_restraint(Model *m,
                                                   RestraintSet *allrs,
                                                   const atom::Hierarchy &h,
                                                   std::string protein,
                                                   int residue, double kappa);

IMPSPBEXPORT void add_diameter_rgyr_restraint(
    RestraintSet *allrs, const atom::Hierarchy &h,
    std::string protein, double diameter, double rgyr, double kappa);

IMPSPBEXPORT IMP::Pointer<spb::EM2DRestraint> em2d_restraint(
    atom::Hierarchies &hs, std::string protein, EM2DParameters EM2D,
    Particle *Sigma);

IMPSPBEXPORT IMP::Pointer<spb::EM2DRestraint> em2d_restraint(
    atom::Hierarchies &hs, std::string protein, EM2DParameters EM2D,
    Floats sigma_grid, Floats fmod_grid);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_SPB_RESTRAINT_H */
