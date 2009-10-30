/**
    *  \file simplify_restraint.cpp
    *  \brief Support for restraints.
    *
    *  Copyright 2007-9 Sali Lab. All rights reserved.
    *
    */

#include "IMP/helper/simplify_restraint.h"
#include "IMP/core/rigid_bodies.h"
#include <IMP/em/DensityMap.h>

IMPHELPER_BEGIN_NAMESPACE

SimpleCollision create_simple_collision_on_rigid_bodies(core::RigidBodies *rbs)
{
  IMP_USAGE_CHECK(rbs->size() > 0, "At least one particle should be given",
     ValueException);

  /****** Set up the nonbonded list ******/
  // Tell nbl on which particles you work on
  // Look for close pairs within the list

  IMP_NEW(core::ListSingletonContainer, lsc, ());
  lsc->add_particles(*rbs);
  IMP_NEW(core::ClosePairsScoreState, nbl, (lsc));

  /****** Refine the nonbonded list ******/
  // Set up the list of close pairs of each pair in NBL
  // because you want to restraint the actual rigid bodies and not its particles
  // Look for close members of the rigid bodies, one from each rigid body

  //if set_close_pairs_finder is not called, then use the non refined list
  //if rcpf omitted, then the score is between 2 spheres
  IMP_NEW(core::RigidClosePairsFinder, rcpf, ());
  nbl->set_close_pairs_finder(rcpf);
  // Set the amount particles need to move before the list is updated
  nbl->set_slack(2);

  /****** Define the score used on each pair in the refined list ******/
  // Score the distance between the spheres of each particles
  // Each particle is required to have x,y,z and a radius
  // The distance is going to be penalized by a harmonic lower bound with
  // mean = 0 and force constant (k) = 1 kcal/mol/A/A
  // The mean and k can be changed later on by calling set_mean and set_k
  // k can also be obtained given the Gaussian standard deviation (angstroms),
  // which can be changed using set_stddev

  IMP_NEW(core::HarmonicLowerBound, h, (0, 1)); // (mean, force constant k)
  IMP_NEW(core::SphereDistancePairScore, sdps, (h));

  /****** Set the restraint ******/
  // Define the restraint, work on all pairs and score them with sdps
  // Add the restraint to the model

  IMP_NEW(core::PairsRestraint, evr, (sdps, nbl->get_close_pairs_container()));

  /****** Add score state and restraint to the model ******/

  Model *mdl = (*rbs)[0].get_model();
  mdl->add_score_state(nbl);
  mdl->add_restraint(evr);

  /****** Return a SimpleCollision object ******/

  return SimpleCollision(evr, h, sdps, nbl);
}

SimpleConnectivity create_simple_connectivity_on_rigid_bodies(
                   core::RigidBodies *rbs)
{
  IMP_USAGE_CHECK(rbs->size() > 0, "At least one particle should be given",
     ValueException);

  /****** Define Refiner ******/
  // Use RigidMembersRefiner when you want the set of particles representing
  // a rigid body to be the same as the set of members

  IMP_NEW(core::RigidMembersRefiner, rmr, ());

  /****** Define PairScore ******/
  // Use RigidBodyDistancePairScore to accelerate computation of the distance
  // between two rigid bodies. The distance is defined as the minimal distance
  // over all bipartite pairs with one particle taken from each rigid body.

  IMP_NEW(core::HarmonicUpperBound, h, (0, 1));
  IMP_NEW(core::SphereDistancePairScore, sdps, (h));
  IMP_NEW(core::RigidBodyDistancePairScore, rdps, (sdps, rmr));

  /****** Set the restraint ******/

  IMP_NEW(core::ConnectivityRestraint, cr, (rdps));
  for ( size_t i=0; i<rbs->size(); ++i )
    cr->set_particles((*rbs)[i].get_particle());

  /****** Add restraint to the model ******/

  Model *mdl = (*rbs)[0].get_model();
  mdl->add_restraint(cr);

  /****** Return a SimpleConnectivity object ******/

  return SimpleConnectivity(cr, h, sdps);
}

SimpleConnectivity create_simple_connectivity_on_molecules(Particles *ps)
{
  IMP_USAGE_CHECK(ps->size() > 0, "At least one particle should be given",
     ValueException);


  /****** Define Refiner ******/
  // Use LeavesRefiner for the hierarchy leaves under a particle

  IMP_NEW(core::LeavesRefiner, lr, (atom::Hierarchy::get_traits()));

  /****** Define PairScore ******/
  // Score on the lowest of the pairs defined by refining the two particles.

  IMP_NEW(core::HarmonicUpperBound, h, (0, 1));
  IMP_NEW(core::SphereDistancePairScore, sdps, (h));
  IMP_NEW(misc::LowestRefinedPairScore, lrps, (lr, sdps));

  /****** Set the restraint ******/

  IMP_NEW(core::ConnectivityRestraint, cr, (lrps));
  cr->set_particles((*ps));

  /****** Add restraint to the model ******/

  Model *mdl = (*ps)[0]->get_model();
  mdl->add_restraint(cr);

  /****** Return a SimpleConnectivity object ******/

  return SimpleConnectivity(cr, h, sdps);
}

SimpleDistance create_simple_distance(Particles *ps)
{
  IMP_USAGE_CHECK(ps->size() == 2, "Two particles should be given",
     ValueException);

  /****** Set the restraint ******/

  IMP_NEW(core::HarmonicUpperBound, h, (0, 1));
  IMP_NEW(core::DistanceRestraint, dr, (h, (*ps)[0], (*ps)[1]));

  /****** Add restraint to the model ******/

  Model *mdl = (*ps)[0]->get_model();
  mdl->add_restraint(dr);

  /****** Return a SimpleDistance object ******/

  return SimpleDistance(dr, h);
}

SimpleDiameter create_simple_diameter(Particles *ps, Float diameter)
{
  IMP_USAGE_CHECK(ps->size() >= 2, "At least two particles should be given",
     ValueException);

  /****** Set the restraint ******/

  IMP_NEW(core::HarmonicUpperBound, h, (0, 1));
  IMP_NEW(core::ListSingletonContainer, lsc, ());
  lsc->add_particles(*ps);
  IMP_NEW(core::DiameterRestraint, dr, (h, lsc, diameter));

  /****** Add restraint to the model ******/

  Model *mdl = (*ps)[0]->get_model();
  mdl->add_restraint(dr);

  /****** Return a SimpleDiameter object ******/

  return SimpleDiameter(dr, h);
}

SimpleExcludedVolume create_simple_excluded_volume_on_rigid_bodies(
                     core::RigidBodies *rbs)
{
  IMP_USAGE_CHECK(rbs->size() > 0, "At least one particle should be given",
     ValueException);

  /****** Set the restraint ******/

  IMP_NEW(core::ListSingletonContainer, lsc, ());
  lsc->add_particles(*rbs);

  IMP_NEW(core::LeavesRefiner, lr, (atom::Hierarchy::get_traits()));
  IMP_NEW(core::ExcludedVolumeRestraint, evr, (lsc, lr));

  /****** Add restraint to the model ******/

  Model *mdl = (*rbs)[0].get_model();
  mdl->add_restraint(evr);

  /****** Return a SimpleExcludedVolume object ******/

  return SimpleExcludedVolume(evr);
}

Particles set_rigid_bodies(atom::Hierarchies const &mhs)
{
  size_t mhs_size = mhs.size();

  IMP_USAGE_CHECK(mhs_size > 0, "At least one hierarchy should be given",
     ValueException);

  Particles rbs;

  for ( size_t i=0; i<mhs_size; ++i )
  {
    // The rigid body is set to be optimized
    IMP::atom::rigid_body_setup_hierarchy(mhs[i]);

    rbs.push_back(mhs[i].get_particle());
  }
  return rbs;
}

em::DensityMap *load_em_density_map(char const *map_fn, float spacing,
                                    float resolution)
{
  em::DensityMap *dmap = em::read_map(map_fn);
  em::DensityHeader *dmap_header = dmap->get_header_writable();
  dmap_header->set_spacing(spacing);
  dmap_header->set_resolution(resolution);

  return dmap;
}

SimpleEMFit create_simple_em_fit(atom::Hierarchies const &mhs,
                                   em::DensityMap *dmap)
{
  size_t mhs_size = mhs.size();

  IMP_USAGE_CHECK(mhs_size > 0, "At least one hierarchy should be given",
     ValueException);

  Particles ps;
  for ( size_t i=0; i<mhs_size; ++i )
  {
    Particles pss = core::get_leaves(mhs[i]);
    for ( size_t j=0; j<pss.size(); ++j )
      ps.push_back(pss[j]);
  }
  IMP_NEW(em::FitRestraint, fit_rs, (ps, dmap,
        core::XYZR::get_default_radius_key(),
        atom::Mass::get_mass_key(),
        1.0));
  Model *mdl = mhs[0].get_particle()->get_model();
  fit_rs->set_model(mdl);
  mdl->add_restraint(fit_rs);

  return SimpleEMFit(fit_rs);
}

IMPHELPER_END_NAMESPACE
