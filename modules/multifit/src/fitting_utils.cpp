/**
 *  \file fitting_utils.cpp
 *  \brief fitting utilities
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/fitting_utils.h>
IMPMULTIFIT_BEGIN_NAMESPACE

em::DensityMap* create_hit_map(core::RigidBody &rb, Refiner *rb_ref,
                            const FittingSolutionRecords &sols,
                            em::DensityMap *dmap
                            ) {
  //get center
  algebra::Vector3D cen =
    core::get_centroid(core::XYZsTemp(rb_ref->get_refined(rb)));
  //create a return map
  Pointer<em::DensityMap> ret(new em::DensityMap(*(dmap->get_header())));
  ret->reset_data(0.);
  algebra::Vector3D cen_t;
  //get all hits
  for(int i=0;i<(int)sols.size();i++){
    cen_t = sols[i].get_fit_transformation().get_transformed(cen);
    if (ret->is_part_of_volume(cen_t)) {
      ret->set_value(cen_t[0],cen_t[1],cen_t[2],ret->get_value(cen_t)+1);
    }
  }
  return ret.release();
}

IMPMULTIFIT_END_NAMESPACE
