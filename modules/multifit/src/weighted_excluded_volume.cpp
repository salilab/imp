/**
 *  \file weighted_excluded_volume.cpp
 *  \brief handles low resolution weighted excluded
 *           volume calculation.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/multifit/weighted_excluded_volume.h>
#include <IMP/em/SurfaceShellDensityMap.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/em/IMPParticlesAccessPoint.h>
#include <IMP/em/MRCReaderWriter.h>
IMPMULTIFIT_BEGIN_NAMESPACE
void add_surface_index(core::Hierarchy mh, Float resolution, Float apix,
                FloatKey shell_key,
                FloatKey radius_key,FloatKey weight_key)  {
  std::cout<<"apix:"<<apix<<" resolution: " << resolution << std::endl;
  Particles ps = core::get_leaves(mh);
  em::IMPParticlesAccessPoint access_p(ps,radius_key,weight_key);
  em::SurfaceShellDensityMap shell_map(access_p,resolution,apix);

  for(unsigned int i=0; i<ps.size(); i++) {
    ps[i]->add_attribute(
      shell_key,shell_map.get_value(core::XYZ(ps[i]).get_coordinates()));
  }
}
IMPMULTIFIT_END_NAMESPACE
