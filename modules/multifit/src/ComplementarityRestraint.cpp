/**
 *  \file WeighedExcludedVolumeRestraint2.cpp
 *  \brief Calculate excluded volume bewteen rigid bodies.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/ComplementarityRestraint.h>
#include <IMP/multifit/internal/GeometricComplementarity.h>
#include <IMP/core/DataObject.h>
#include <IMP/log.h>

IMPMULTIFIT_BEGIN_NAMESPACE
IMP::algebra::DenseGrid3D<float>
ComplementarityRestraint::get_grid(const ParticlesTemp &a,
                                           double thickness,
                                           double value,
                                           double interior_thickness,
                                           double voxel) const {
  internal::ComplementarityGridParameters params;
  params.complementarity_thickness=Floats(1, thickness);
  params.complementarity_value=Floats(1, value);
  params.interior_thickness=interior_thickness;
  params.interior_cutoff_distance= maximum_penetration_;
  params.voxel_size=voxel;
  IMP::algebra::DenseGrid3D<float> grid
    = internal::get_complementarity_grid(a, params);
  return grid;
}

ComplementarityRestraint::GridObject *
ComplementarityRestraint::get_grid_object(core::RigidBody rb,
                                                  const ParticlesTemp &a,
                                                  ObjectKey ok,
                                                  double thickness,
                                                  double value,
                                                  double interior_thickness,
                                                  double voxel) const {
    IMP_USAGE_CHECK(!a.empty(), "No particles passed for excluded volume");
    for (unsigned int i=1; i< a.size(); ++i) {
      IMP_USAGE_CHECK(core::RigidMember(a[0]).get_rigid_body()
                      == core::RigidMember(a[i]).get_rigid_body(),
                      "Not all particles are from the same rigid body.");
    }
    if (!rb->has_attribute(ok)) {
      IMP_LOG_TERSE( "Creating grid for rigid body " << rb->get_name()
          << std::endl);
      IMP::algebra::DenseGrid3D<float> grid
        = get_grid(a, thickness, value, interior_thickness, voxel);
      IMP_LOG_TERSE( "Grid has size " << grid.get_number_of_voxels(0)
              << ", " << grid.get_number_of_voxels(1)
              << ", " << grid.get_number_of_voxels(2)
              << std::endl);
      Pointer<GridObject> n(new GridObject(GridPair(rb.get_reference_frame()
                                                    .get_transformation_to(),
                                                    grid)));
      rb->add_cache_attribute(ok, n);
    }
    IMP_CHECK_OBJECT(rb->get_value(ok));
    IMP_INTERNAL_CHECK(dynamic_cast<GridObject*>(rb->get_value(ok)),
                       "The saved grid is not a grid.");
    return dynamic_cast<GridObject*>(rb->get_value(ok));
  }
ComplementarityRestraint
::ComplementarityRestraint(
                                   const ParticlesTemp &a,
                                   const ParticlesTemp &b,
                                   std::string name)
 :
    Restraint(IMP::internal::get_model(a), name), a_(a), b_(b),
  rba_(core::RigidMember(a[0]).get_rigid_body()),
  rbb_(core::RigidMember(b[0]).get_rigid_body()),
  ok_("wev grid"),
  maximum_separation_(std::numeric_limits<double>::max()),
  maximum_penetration_score_(std::numeric_limits<double>::max()),
  maximum_penetration_(std::numeric_limits<double>::max()),
  complementarity_thickness_(10), complementarity_value_(-1),
  penetration_coef_(2), complementarity_coef_(1),
  boundary_coef_(-3),
  interior_thickness_(2){
  update_voxel();
}

double ComplementarityRestraint::unprotected_evaluate(
                               DerivativeAccumulator *accum) const
{
  return unprotected_evaluate_if_good(accum,
                                      std::numeric_limits<double>::max());
}


namespace
{

bool score_acceptable(const IMP::multifit::internal::FitScore &ps)
{
  return ps.penetration_score < std::numeric_limits<float>::max() &&
    (ps.penetration_score != 0 || ps.complementarity_score != 0 ||
     ps.boundary_score != 0);
}

}


double ComplementarityRestraint::unprotected_evaluate_if_good(
                                       DerivativeAccumulator *accum,
                                       double max) const
{
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK_VARIABLE(accum);
  IMP_USAGE_CHECK(!accum,
                  "ComplementarityRestraint does not support derivatives.");
  double vol= cube(voxel_size_);

  internal::ComplementarityParameters params;
  params.maximum_separation= maximum_separation_;
  params.maximum_penetration_score= std::min(maximum_penetration_score_/vol,
                                             max);
  //std::cout<<"max penet score:"<<params.maximum_penetration_score<<"(" <<
  //maximum_penetration_score_<<","<<vol<<","<<max<<")"<<std::endl;
  Pointer<GridObject> ga=get_grid_object(rba_, a_, ok_,
                                         complementarity_thickness_,
                                         complementarity_value_,
                                         interior_thickness_, voxel_size_);
  Pointer<GridObject> gb=get_grid_object(rbb_, b_, ok_,
                                         complementarity_thickness_,
                                         complementarity_value_,
                                         interior_thickness_, voxel_size_);
  algebra::Transformation3D tra=ga->get_data().first
    *rba_.get_reference_frame().get_transformation_from();
  algebra::Transformation3D trb=rbb_.get_reference_frame()
    .get_transformation_to()
    /gb->get_data().first;
  // transform a by tra and b by trb
  // same as transforming b by na/oa Ma= oa/ nai nb/ob p
  algebra::Transformation3D tr= tra*trb;
  IMP_LOG_TERSE( "Transformation is " << tr << " between "
          << rba_.get_reference_frame()
          << " and " << rbb_.get_reference_frame() << std::endl);

  IMP::multifit::internal::FitScore ps= IMP::multifit::internal
    ::get_fit_scores(ga->get_data().second,
                     gb->get_data().second,
                     tr, params);
  IMP_LOG_TERSE( "Scores are " << ps.penetration_score << ", "
      << ps.complementarity_score << " and "
      << ps.boundary_score
          << std::endl);
  /*  std::cout<<"Scores are " << ps.penetration_score << ", "
      << ps.complementarity_score << " and "
      << ps.boundary_score
      << std::endl;*/
  if ( !score_acceptable(ps) ) {
    //    std::cout<<"scores are not accetable"<<std::endl;
    return std::numeric_limits<double>::max();
  }
  double score = penetration_coef_*ps.penetration_score
    + complementarity_coef_*ps.complementarity_score
    + boundary_coef_*ps.boundary_score;
  return score*vol;
}


void ComplementarityRestraint::update_voxel() {
  double val;
  if (complementarity_thickness_==0) {
    val= interior_thickness_;
  } else if (interior_thickness_==0) {
    val= complementarity_thickness_;
  } else {
    val=std::min(complementarity_thickness_,
                 interior_thickness_);
  }
  voxel_size_=val/2.0;
}

ParticlesTemp ComplementarityRestraint::get_input_particles() const
{
  ParticlesTemp ret= a_;
  ret.insert(ret.end(), b_.begin(), b_.end());
  ret.push_back(rba_);
  ret.push_back(rbb_);
  return ret;
}

ContainersTemp ComplementarityRestraint::get_input_containers() const {
  return ContainersTemp();
}

void ComplementarityRestraint::do_show(std::ostream& out) const
{
  out<<get_name()<<" with parameters:"<<
    " maximum_separation_: "<<maximum_separation_<<
    ", maximum_penetration_score: "<<maximum_penetration_score_<<
    ", maximum_penetration: "<<maximum_penetration_<<
    ", complementarity_thickness: "<<complementarity_thickness_<<
    ", complementarity_value: "<<complementarity_value_<<
    ", penetration_coef: "<<penetration_coef_<<
    ", boundary_coef: "<<boundary_coef_<<
    ", interior_thickness: "<<interior_thickness_<<
    ", voxel_size: "<<voxel_size_<<std::endl;
}

IMPMULTIFIT_END_NAMESPACE
