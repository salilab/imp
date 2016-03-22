/**
 *  \file IMP/pmi/TransformMover.h
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPPMI_TRANSFORM_MOVER_H
#define IMPPMI_TRANSFORM_MOVER_H

#include <IMP/pmi/pmi_config.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/XYZ.h>
IMPPMI_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPPMIEXPORT TransformMover : public IMP::core::MonteCarloMover {
  algebra::Transformation3D last_transformation_;
  Float max_translation_;
  Float max_angle_;
  IMP::ParticleIndexes pixyzs_;
  IMP::ParticleIndexes pirbs_;
  IMP::ParticleIndexes pis_;
  IMP::algebra::Transformation3D t_;
  IMP::algebra::Transformation3D c_;
  IMP::algebra::Transformation3Ds rbts_;
  IMP::algebra::Vector3Ds xyzs_;
  IMP::algebra::Vector3D axis_;
  IMP::algebra::Transformation3D tt_;
  unsigned int constr_;

IMP::algebra::Vector3D get_center(){
  Float x=0;
  Float y=0;
  Float z=0;
  unsigned int nelements=0;
  for (unsigned int i=0;i<pixyzs_.size();i++) {
       core::XYZ d(get_model(), pixyzs_[i]);
       algebra::Vector3D xyz=d.get_coordinates();
       x=x+xyz[0];
       y=y+xyz[1];
       z=z+xyz[2];
       nelements++;
  }

  for (unsigned int i=0;i<pirbs_.size();i++){
      core::RigidBody rb(get_model(), pirbs_[i]);
      algebra::Vector3D xyz=rb.get_coordinates();
      unsigned int nparticles=rb.get_number_of_members();
      x=x+xyz[0]*float(nparticles);
      y=y+xyz[1]*float(nparticles);
      z=z+xyz[2]*float(nparticles);
      nelements=nelements+nparticles;      
  }

  algebra::Vector3D center;
  center[0]=x/float(nelements);
  center[1]=y/float(nelements);  
  center[2]=z/float(nelements);
  return center;
}


 public:
  TransformMover(Model *m, Float max_translation, Float max_rotation);

  TransformMover(Model *m, algebra::Vector3D axis, 
                                   Float max_translation, Float max_rotation);


void add_xyz_particle(IMP::ParticleIndexAdaptor pi){
if ( core::RigidBody::get_is_setup(get_model(), pi) ) {
    pirbs_.push_back(pi);
    pis_.push_back(pi); }
else {
    pixyzs_.push_back(pi);
    pis_.push_back(pi); }
}

void add_rigid_body_particle(IMP::ParticleIndexAdaptor pi){
pirbs_.push_back(pi);
pis_.push_back(pi);
//initializing the last_transformation array
//last_transformation_.push_back( d.get_reference_frame().get_transformation_to());
}



  void set_maximum_translation(Float mt) {
    IMP_USAGE_CHECK(mt > 0, "Max translation must be positive");
    max_translation_ = mt;
  }

  void set_maximum_rotation(Float mr) {
    IMP_USAGE_CHECK(mr > 0, "Max rotation must be positive");
    max_angle_ = mr;
  }

  Float get_maximum_translation() const { return max_translation_; }

  Float get_maximum_rotation() const { return max_angle_; }

  IMP::algebra::Transformation3D get_last_transformation() const { return tt_; }

 protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(TransformMover);
};

IMPPMI_END_NAMESPACE

#endif /* IMPPMI_TRANSFORM_MOVER_H */
