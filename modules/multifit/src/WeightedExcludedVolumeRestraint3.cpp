/**
 *  \file WeighedExcludedVolumeRestraint2.cpp
 *  \brief Calculate excluded volume bewteen rigid bodies.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/WeightedExcludedVolumeRestraint3.h>
#include <IMP/multifit/internal/GeometricComplementarity.h>
#include <IMP/core/DataObject.h>
#include <IMP/log.h>

IMPMULTIFIT_BEGIN_NAMESPACE

namespace {
  typedef core::DataObject<algebra::DenseGrid3D<float> > GridObject;
  GridObject *get_grid_object(const ParticlesTemp &a, ObjectKey ok,
                              double thickness, double value,
                              double interior_thickness,
                              double voxel) {
    IMP_USAGE_CHECK(!a.empty(), "No particles passed for excluded volume");
    for (unsigned int i=1; i< a.size(); ++i) {
      IMP_USAGE_CHECK(core::RigidMember(a[0]).get_rigid_body()
                      == core::RigidMember(a[i]).get_rigid_body(),
                      "Not all particles are from the same rigid body.");
    }
    core::RigidBody rb= core::RigidMember(a[0]).get_rigid_body();
    if (!rb->has_attribute(ok)) {
      internal::ComplementarityGridParameters params;
      params.complementarity_thickness=Floats(1, thickness);
      params.complementarity_value=Floats(1, value);
      params.interior_thickness=interior_thickness;
      params.voxel_size=voxel;
      IMP::algebra::DenseGrid3D<float> grid
        = internal::get_complentarity_grid(a, params);
      Pointer<GridObject> n(new GridObject(grid));
      rb->add_cache_attribute(ok, n);
    }
    IMP_CHECK_OBJECT(rb->get_value(ok));
    return dynamic_cast<GridObject*>(rb->get_value(ok));
  }
}
WeightedExcludedVolumeRestraint3
::WeightedExcludedVolumeRestraint3(
                                   const ParticlesTemp &a,
                                   const ParticlesTemp &b,
                                   std::string name)
 :
  Restraint(name), a_(a), b_(b),
  rba_(core::RigidMember(a[0]).get_rigid_body()),
  rbb_(core::RigidMember(b[0]).get_rigid_body()),
  ok_("wev grid"),
  maximum_separation_(std::numeric_limits<double>::max()),
  maximum_penetration_score_(std::numeric_limits<double>::max()),
  complementarity_thickeness_(10), complementarity_value_(-1),
  interior_thickness_(2){
  update_voxel();
}

//! A value of 1 means that the proteins are penetrating
//! A value of 0 means that the proteins are not touching.
//! A negative value is the geometric complimintarity
double WeightedExcludedVolumeRestraint3::unprotected_evaluate(
                         DerivativeAccumulator *accum) const
{
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(!accum,
                  "WeightedExcludedVolume3 does not support derivatives.");
  algebra::Transformation3D tr
    = rba_.get_reference_frame().get_transformation_from()
    *rba_.get_reference_frame().get_transformation_to();
  internal::ComplementarityParameters params;
  params.maximum_separation= maximum_separation_;
  params.maximum_penetration_score= maximum_penetration_score_;

  Pointer<GridObject> ga=get_grid_object(a_, ok_,
                                         complementarity_thickeness_,
                                         complementarity_value_,
                                         interior_thickness_, voxel_size_);
  Pointer<GridObject> gb=get_grid_object(b_, ok_,
                                         complementarity_thickeness_,
                                         complementarity_value_,
                                         interior_thickness_, voxel_size_);
  FloatPair ps= IMP::multifit::internal
    ::get_penetration_and_complementarity_scores(ga->get_data(),
                                                 gb->get_data(),
                                                 tr, params);
  double vol= cube(voxel_size_);
  if (ps.first==ps.second && ps.second==0) {
    return std::numeric_limits<double>::max();
  } else if (ps.first < maximum_penetration_score_) return ps.second/vol;
  else return ps.first/vol;
}

void WeightedExcludedVolumeRestraint3::update_voxel() {
  double val;
  if (complementarity_thickeness_==0) {
    val= interior_thickness_;
  } else if (interior_thickness_==0) {
    val= complementarity_thickeness_;
  } else {
    val=std::min(complementarity_thickeness_,
                 interior_thickness_);
  }
  voxel_size_=val/2.0;
}

ParticlesTemp WeightedExcludedVolumeRestraint3::get_input_particles() const
{
  ParticlesTemp ret= a_;
  ret.insert(ret.end(), b_.begin(), b_.end());
  ret.push_back(rba_);
  ret.push_back(rbb_);
  return ret;
}

ContainersTemp WeightedExcludedVolumeRestraint3::get_input_containers() const {
  return ContainersTemp();
}

void WeightedExcludedVolumeRestraint3::do_show(std::ostream& out) const
{
  out<<"WeightedExcludedVolumeRestraint3"<<std::endl;
}

IMPMULTIFIT_END_NAMESPACE
