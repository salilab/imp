/**
 *  \file weighted_excluded_volume.cpp
 *  \brief handles low resolution weighted excluded
 *           volume calculation.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/weighted_excluded_volume.h>
#include <IMP/em/SurfaceShellDensityMap.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/CloseBipartitePairContainer.h>
#include <IMP/core/RigidClosePairsFinder.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/core/HarmonicLowerBound.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/em/MRCReaderWriter.h>
IMPMULTIFIT_BEGIN_NAMESPACE
void add_surface_index(core::Hierarchy mh,Float apix,
                FloatKey shell_key,
                FloatKey ,FloatKey )  {
  ParticlesTemp ps = core::get_leaves(mh);
  base::Pointer<em::SurfaceShellDensityMap> shell_map
    = new em::SurfaceShellDensityMap(ps,apix);

  for(unsigned int i=0; i<ps.size(); i++) {
    IMP_INTERNAL_CHECK(! ps[i]->has_attribute(shell_key),
     "Particle " << ps[i]->get_name() <<
     " already has shell attribute" << std::endl);
    ps[i]->add_attribute(
      shell_key,shell_map->get_value(core::XYZ(ps[i]).get_coordinates()));
  }
}

IMP::Restraint *create_weighted_excluded_volume_restraint(
   core::RigidBody rb1,
   core::RigidBody rb2,
   FloatKey ) {
  IMP::Model* mdl=rb1.get_particle()->get_model();
  //generate the list singleton contrainers
  IMP_NEW(core::LeavesRefiner,leaves_refiner,(atom::Hierarchy::get_traits()));
  ParticlesTemp ps1= leaves_refiner->get_refined(rb1),
    ps2= leaves_refiner->get_refined(rb2);
  IMP_NEW(container::ListSingletonContainer,ls1,(ps1));
  IMP_NEW(container::ListSingletonContainer,ls2,(ps2));
  //set up the nonbonded list
  IMP_NEW(core::RigidClosePairsFinder, close_pair_finder,());
  IMP_NEW(container::CloseBipartitePairContainer,nbl,
          (ls1,ls2,2.,close_pair_finder));
  //Set up excluded volume
  IMP_NEW(core::HarmonicLowerBound,hlb,(0,1));
  IMP_NEW(core::SphereDistancePairScore,ps,(hlb));
  IMP_NEW(container::PairsRestraint,evr,(ps, nbl));
  mdl->add_restraint(evr);
  return evr;
}
IMPMULTIFIT_END_NAMESPACE
