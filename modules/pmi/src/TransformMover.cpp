/**
 *  \file TransformMover.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/pmi/TransformMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/random.h>
#include <IMP/algebra/vector_generators.h>
IMPPMI_BEGIN_NAMESPACE

TransformMover::TransformMover(Model *m,
                               Float max_translation, Float max_angle)
    : MonteCarloMover(m, "Transform mover") {
  IMP_LOG_VERBOSE("start TransformMover constructor");
  max_translation_ = max_translation;
  max_angle_ = max_angle;
  constr_=0;
  called_=0;
  not_accepted_=0;
  IMP_LOG_VERBOSE("finish mover construction" << std::endl);
}

TransformMover::TransformMover(Model *m, algebra::Vector3D axis,
                               Float max_translation, Float max_angle)
    : MonteCarloMover(m, "Transform mover") {
  IMP_LOG_VERBOSE("start TransformMover constructor");
  //this constructor defines a 2D rotation about an axis
  axis_ = axis;
  max_translation_ = max_translation;
  max_angle_ = max_angle;
  constr_=1;
  called_=0;
  not_accepted_=0;
  IMP_LOG_VERBOSE("finish mover construction" << std::endl);
}


TransformMover::TransformMover(Model *m, IMP::ParticleIndexAdaptor p1i, IMP::ParticleIndexAdaptor p2i,
                               Float max_translation, Float max_angle)
    : MonteCarloMover(m, "Transform mover") {
  IMP_LOG_VERBOSE("start TransformMover constructor");
  //this constructor defines a rotation about an axis defined by two particles
  p1i_ = p1i;
  p2i_ = p2i;
  max_translation_ = max_translation;
  max_angle_ = max_angle;
  constr_=2;
  called_=0;
  not_accepted_=0;
  IMP_LOG_VERBOSE("finish mover construction" << std::endl);
}



core::MonteCarloMoverResult TransformMover::do_propose() {
  IMP_OBJECT_LOG;

  xyzs_.resize(pixyzs_.size());
  rbts_.resize(pirbs_.size());

  algebra::Vector3D translation = IMP::algebra::Vector3D(0,0,0);

  if (constr_==0) {
    algebra::Vector3D xyzc=get_center(); //check the correct type from algebra::get_unit_sphere_d<3>()
    c_=IMP::algebra::Transformation3D(xyzc);
    translation = algebra::get_random_vector_in(
    algebra::Sphere3D(algebra::get_zero_vector_d<3>(), max_translation_));
    axis_=algebra::get_random_vector_on(algebra::get_unit_sphere_d<3>());

  } else if (constr_==1) {
    algebra::Vector3D xyzc=get_center(); //check the correct type from algebra::get_unit_sphere_d<3>()
    c_=IMP::algebra::Transformation3D(xyzc);
    translation = algebra::get_random_vector_in(
      algebra::Sphere3D(algebra::get_zero_vector_d<3>(), max_translation_));

  } else if (constr_==2) {
    core::XYZ d1=IMP::core::XYZ(get_model(),p1i_);
    core::XYZ d2=IMP::core::XYZ(get_model(),p2i_);
    c_=IMP::algebra::Transformation3D(d1.get_coordinates());

    axis_=(d2.get_coordinates()-d1.get_coordinates()).get_unit_vector();
  }

  ::boost::uniform_real<> rand(-max_angle_, max_angle_);
  Float angle = rand(random_number_generator);
  algebra::Rotation3D r = algebra::get_rotation_about_axis(axis_, angle);
  algebra::Transformation3D t(r, translation);

  algebra::Transformation3D tt = c_*t*c_.get_inverse();
  tt_ = tt;

  for (unsigned int i=0;i<pixyzs_.size();i++) {
    core::XYZ d(get_model(), pixyzs_[i]);
    xyzs_[i]=d.get_coordinates();
    core::transform(d,tt);
  }

  for (unsigned int i=0;i<pirbs_.size();i++){
    core::RigidBody d(get_model(), pirbs_[i]);
    rbts_[i]=d.get_reference_frame().get_transformation_to();
    core::transform(d,tt);
  }

  return core::MonteCarloMoverResult(pis_, 1.0);
}

void TransformMover::do_reject() {
  not_accepted_++;
  for (unsigned int i=0;i<pixyzs_.size();i++) {
    core::XYZ d(get_model(), pixyzs_[i]);
    d.set_coordinates(xyzs_[i]);
  }

  for (unsigned int i=0;i<pirbs_.size();i++){
    core::RigidBody d(get_model(), pirbs_[i]);
    d.set_reference_frame(algebra::ReferenceFrame3D(rbts_[i]));
  }

}

ModelObjectsTemp TransformMover::do_get_inputs() const {
  ModelObjectsTemp ret(pis_.size());
  for (unsigned int i = 0; i < pis_.size(); ++i) {
    ret[i] = get_model()->get_particle(pis_[i]);
  }
  return ret;
}

IMPPMI_END_NAMESPACE
