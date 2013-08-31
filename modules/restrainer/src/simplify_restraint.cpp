/**
    *  \file simplify_restraint.cpp
    *  \brief Support for restraints.
    *
    *  Copyright 2007-2013 IMP Inventors. All rights reserved.
    *
    */

#include "IMP/restrainer/simplify_restraint.h"
#include "IMP/core/rigid_bodies.h"
#include <IMP/em/DensityMap.h>

IMPRESTRAINER_BEGIN_NAMESPACE

SimpleConnectivity create_simple_connectivity_on_rigid_bodies(
                                       const core::RigidBodies &rbs,
                                       Refiner *ref)
{
  IMP_USAGE_CHECK(rbs.size() > 0, "At least one particle should be given");

  /****** Define PairScore ******/
  // Use RigidBodyDistancePairScore to accelerate computation of the distance
  // between two rigid bodies. The distance is defined as the minimal distance
  // over all bipartite pairs with one particle taken from each rigid body.

  IMP_NEW(core::HarmonicUpperBound, h, (0, 1));
  IMP_NEW(core::SphereDistancePairScore, sdps, (h));
  IMP_NEW(core::RigidBodyDistancePairScore, rdps, (sdps, ref));

  /****** Set the restraint ******/

  IMP_NEW(core::ConnectivityRestraint, cr, (rdps));
  for ( size_t i=0; i<rbs.size(); ++i )
    //cr->set_particles((*rbs)[i].get_particle());
    cr->add_particle(rbs[i].get_particle());

  /****** Add restraint to the model ******/

  //Model *mdl = rbs[0].get_model();
  //mdl->add_restraint(cr);

  /****** Return a SimpleConnectivity object ******/

  return SimpleConnectivity(cr, h, sdps);
}

SimpleConnectivity create_simple_connectivity_on_molecules(
                   const atom::Hierarchies &mhs)
{
  size_t mhs_size = mhs.size();

  IMP_USAGE_CHECK(mhs_size > 0, "At least one hierarchy should be given");

  kernel::ParticlesTemp ps;

  for ( size_t i=0; i<mhs_size; ++i )
  {
    ps.push_back(mhs[i].get_particle());
  }

  /****** Define Refiner ******/
  // Use LeavesRefiner for the hierarchy leaves under a particle

  IMP_NEW(core::LeavesRefiner, lr, (atom::Hierarchy::get_traits()));

  /****** Define PairScore ******/
  // Score on the lowest of the pairs defined by refining the two particles.

  IMP_NEW(core::HarmonicUpperBound, h, (0, 1));
  IMP_NEW(core::SphereDistancePairScore, sdps, (h));
  IMP_NEW(core::KClosePairsPairScore, lrps, (sdps, lr));

  /****** Set the restraint ******/

  IMP_NEW(core::ConnectivityRestraint, cr, (lrps));
  cr->set_particles((ps));

  /****** Add restraint to the model ******/

  //Model *mdl = mhs[0].get_particle()->get_model();
  //mdl->add_restraint(cr);

  /****** Return a SimpleConnectivity object ******/

  return SimpleConnectivity(cr, h, sdps);
}

SimpleDistance create_simple_distance(const kernel::Particles &ps)
{
  IMP_USAGE_CHECK(ps.size() == 2, "Two particles should be given");

  /****** Set the restraint ******/

  IMP_NEW(core::HarmonicUpperBound, h, (0, 1));
  IMP_NEW(core::DistanceRestraint, dr, (h, ps[0], ps[1]));

  /****** Add restraint to the model ******/

  //Model *mdl = (*ps)[0]->get_model();
  //mdl->add_restraint(dr);

  /****** Return a SimpleDistance object ******/

  return SimpleDistance(dr, h);
}

SimpleDiameter create_simple_diameter(const kernel::Particles &ps, Float diameter)
{
  IMP_USAGE_CHECK(ps.size() >= 2, "At least two particles should be given");

  /****** Set the restraint ******/

  IMP_NEW(core::HarmonicUpperBound, h, (0, 1));
  IMP_NEW(container::ListSingletonContainer, lsc, (get_as<kernel::ParticlesTemp>(ps)));
  IMP_NEW(core::DiameterRestraint, dr, (h, lsc, diameter));

  /****** Add restraint to the model ******/

  //Model *mdl = (*ps)[0]->get_model();
  //mdl->add_restraint(dr);

  /****** Return a SimpleDiameter object ******/

  return SimpleDiameter(dr, h);
}

SimpleExcludedVolume create_simple_excluded_volume_on_rigid_bodies(
                                         const core::RigidBodies &rbs,
                                         Refiner *ref)
{
  IMP_USAGE_CHECK(rbs.size() > 0, "At least one particle should be given");

  /****** Set the restraint ******/

  kernel::ParticlesTemp all;
  for (unsigned int i=0; i< rbs.size(); ++i) {
    kernel::ParticlesTemp cur= ref->get_refined(rbs[i]);
    all.insert(all.end(), cur.begin(), cur.end());
  }

  IMP_NEW(container::ListSingletonContainer, lsc, (all));

  IMP_NEW(core::ExcludedVolumeRestraint, evr, (lsc));

  /****** Add restraint to the model ******/

  //Model *mdl = rbs[0].get_model();
  //mdl->add_restraint(evr);

  /****** Return a SimpleExcludedVolume object ******/

  return SimpleExcludedVolume(evr);
}

SimpleExcludedVolume create_simple_excluded_volume_on_molecules(
                     atom::Hierarchies const &mhs)
{
  IMP_CHECK_CODE(size_t mhs_size = mhs.size());

  IMP_USAGE_CHECK(mhs_size > 0, "At least one hierarchy should be given");

  kernel::ParticlesTemp ps;

  for ( size_t i=0; i<mhs.size(); ++i )
  {
    kernel::ParticlesTemp leaves= IMP::atom::get_leaves(mhs[i]);
    ps.insert(ps.end(), leaves.begin(), leaves.end());
  }

  /****** Set the restraint ******/

  IMP_NEW(container::ListSingletonContainer, lsc, (ps));

  IMP_NEW(core::ExcludedVolumeRestraint, evr, (lsc));

  /****** Add restraint to the model ******/

  //Model *mdl = mhs[0].get_particle()->get_model();
  //mdl->add_restraint(evr);

  /****** Return a SimpleExcludedVolume object ******/

  return SimpleExcludedVolume(evr);
}

core::RigidBodies set_rigid_bodies(atom::Hierarchies const &mhs)
{
  IMPRESTRAINER_DEPRECATED_FUNCTION_DEF(2.1,
                                      "Use atom::create_rigid_body() instead.");
  size_t mhs_size = mhs.size();

  IMP_USAGE_CHECK(mhs_size > 0, "At least one hierarchy should be given");

  kernel::Particles rbps;

  for ( size_t i=0; i<mhs_size; ++i )
  {
    // The rigid body is set to be optimized
    IMP::atom::setup_as_rigid_body(mhs[i]);
    rbps.push_back(mhs[i].get_particle());
  }
  return (core::RigidBodies(rbps));
}
SimpleEMFit
  create_simple_em_fit(atom::Hierarchy const &mh, em::DensityMap *dmap) {
  atom::Hierarchies mhs;
  mhs.push_back(mh);
  return create_simple_em_fit(mhs,dmap);
}

em::DensityMap *load_em_density_map(char const *map_fn, float spacing,
                                    float resolution)
{
  em::DensityMap *dmap = em::read_map(map_fn);
  em::DensityHeader *dmap_header = dmap->get_header_writable();
  dmap->update_voxel_size(spacing);
  dmap_header->set_resolution(resolution);
  return dmap;
}

SimpleEMFit create_simple_em_fit(atom::Hierarchies const &mhs,
                                   em::DensityMap *dmap)
{
  size_t mhs_size = mhs.size();

  IMP_USAGE_CHECK(mhs_size > 0, "At least one hierarchy should be given");
  kernel::ParticlesTemp ps;
  for ( size_t i=0; i<mhs_size; ++i )
  {
    kernel::ParticlesTemp pss = core::get_leaves(mhs[i]);
    ps.insert(ps.end(),pss.begin(),pss.end());
  }

  IMP_NEW(em::FitRestraint, fit_rs, (ps, dmap,
        FloatPair(0.,0.),
        atom::Mass::get_mass_key(),
        1.0));
  return SimpleEMFit(fit_rs);
}

IMPRESTRAINER_END_NAMESPACE
