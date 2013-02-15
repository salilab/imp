/**
 *  \file RigidBodiesImageFitRestraint
 *  \brief
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/RigidBodiesImageFitRestraint.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/em2d/project.h"
#include "IMP/algebra/utility.h"
#include "IMP/algebra/VectorD.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/Transformation3D.h"
#include "IMP/atom/Mass.h"
#include <IMP/log.h>
#include "IMP/exception.h"
#include "IMP/Particle.h"
#include <IMP/SingletonContainer.h>
#include "IMP/container.h"
#include <math.h>
#include <iostream>


IMPEM2D_BEGIN_NAMESPACE

RigidBodiesImageFitRestraint::RigidBodiesImageFitRestraint(
                          ScoreFunction *scf,
                          const core::RigidBodies &rbs,
                          Image *img): score_function_(scf),
                          rigid_bodies_(rbs),
                          image_(img),
                          params_set_(false) {
  maps_.resize(rbs.size());
  image_->set_was_used(true);
  rigid_bodies_masks_.resize(rbs.size());
  projection_ = new Image();
  projection_->set_size(img);
  projection_->set_was_used(true);
  IMP_LOG_TERSE( "RigidBodiesImageFitRestraint: Image for projection "
          "created. Size: " << projection_->get_data().rows << "x"
          << projection_->get_data().cols << std::endl);
}

double
RigidBodiesImageFitRestraint::unprotected_evaluate(
                                          DerivativeAccumulator *accum) const {
  IMP_UNUSED(accum);
  IMP_USAGE_CHECK(!accum, "No derivatives provided");
  IMP_LOG_TERSE( "RigidBodiesImageFitRestraint::unprotected_evaluate "
           << "rigid bodies " << rigid_bodies_.size() <<std::endl);

  projection_->set_zeros();
  // Form the projection with the positions of the rigid bodies
  for (unsigned int i=0; i < rigid_bodies_.size(); ++i) {
    algebra::Transformation3D T =
                rigid_bodies_[i].get_reference_frame().get_transformation_to();
    Ints key = get_unique_index( T.get_rotation() );

    // Get the precomputed projection mask for the rotation
    KeyIndexMap::const_iterator it = maps_[i].find(key);
    IMP_USAGE_CHECK(it != maps_[i].end(),
                          "Key corresponding to the rotation not found");
    unsigned int index = (*it).second;

    // Place in the matrix
    algebra::Vector3D t = T.get_translation();
    algebra::Vector2D shift(t[0]/params_.pixel_size, t[1]/params_.pixel_size);
    do_place(rigid_bodies_masks_[i][index]->get_data() ,
             projection_->get_data(), shift);
  }

  /***************************
  IMP_NEW(SpiderReaderWriter,srw,());
  projection_->write("composed_projection.spi",srw);
  ***************************/
  do_normalize(projection_->get_data());
  return score_function_->get_score(image_, projection_);
}


void RigidBodiesImageFitRestraint::set_orientations(const core::RigidBody &rb,
                            const algebra::Rotation3Ds &rots) {

  IMP_LOG_TERSE( "Setting rotations for " << rb->get_name() << std::endl);

  IMP_USAGE_CHECK(params_set_ == true,
      "RigidBodiesImageFitRestraint: Parameters for projecting are not set");

  unsigned int j = get_rigid_body_index(rb);

  // Get the particles of the rigid body
  core::RigidMembers rbm = rb.get_members();
  ParticlesTemp ps;

  for (unsigned int i=0; i < rbm.size(); ++i) {
    // Discard particles that do not have mass
    if (atom::Mass::particle_is_instance(rbm[i].get_particle()) ) {
      ps.push_back(rbm[i].get_particle());
    }
  }
  unsigned int size = get_enclosing_image_size(ps, params_.pixel_size, 4);

  // Project the rigid body with each rotation
  Images masks(rots.size());
  RegistrationResults regs;
  KeyIndexMap kmap;
  algebra::Transformation3D T =rb.get_reference_frame().get_transformation_to();
  // R0 is the initial rotation of the rigid body in its reference frame
  algebra::Rotation3D R0 = T.get_rotation();
  algebra::Rotation3D R0i = R0.get_inverse();

  for (unsigned int i=0; i < rots.size(); ++i) {
    // I treat the orientations in the parameter rots as absolute rotations of
    // the reference frames.
    // To project in the direction the absolute reference frame rot[i], first
    // I need to find the relative rotation to apply to the object, as it
    // has a reference frame already. The solution is simple: first invert
    // the rotation in the reference frame of the object (R0i), and then apply
    // the absolute rotation desired (rot[i]).
    Ints ints = get_unique_index(rots[i]);
    algebra::Rotation3D rotation_for_projecting=algebra::compose(rots[i], R0i);
    algebra::Vector2D v(0., 0.);
    RegistrationResult reg(rotation_for_projecting, v);
    regs.push_back(reg);
    kmap.insert ( KeyIndexPair(ints, i) );
  }
  /***************************/

   ProjectingOptions options(params_.pixel_size, params_.resolution);
   options.normalize = false; // Summing normalized masks does not make sense
   masks = get_projections(ps, regs, size, size, options);

   /***************************
   IMP_NEW(SpiderImageReaderWriter,srw,());
   for (unsigned int k=0; k< masks.size(); ++k) {
     std::ostringstream oss;
     oss << "rigid-body-" << j << "-mask-" << k << ".spi";
     masks[k]->write(oss.str(), srw);
   }
   ***************************/

  maps_[j] = kmap;
  rigid_bodies_masks_[j] = masks;
}


void RigidBodiesImageFitRestraint::set_projecting_parameters(
                                            const ProjectingParameters &p) {
  params_ = p;
  params_set_ = true;
}


ParticlesTemp RigidBodiesImageFitRestraint::get_input_particles() const
{
  ParticlesTemp ret;
  for (unsigned int i=0; i < rigid_bodies_.size(); ++i) {
    ret.push_back( rigid_bodies_[i].get_particle());
  }
  return ret;
}

ContainersTemp RigidBodiesImageFitRestraint::get_input_containers() const
{
  ContainersTemp ct;
  return ct;
}

Ints get_unique_index(const algebra::Rotation3D &rot) {
  Ints unique(4);
  algebra::Vector4D v = rot.get_quaternion();
  v = (v[0] > 0) ? v : (-1)*v;
  // form the unique 4 ints ( 2 first decimal positions)
  for (unsigned int i=0; i < unique.size(); ++i) {
    unique[i] = floor(100*v[i]);
  }
  IMP_LOG_TERSE( "get_unique_index: quaternion " << v
          << " index " << unique[0] << " " << unique[1] << " " << unique[2]
          << " " << unique[3] << std::endl);
  return unique;
}

  void RigidBodiesImageFitRestraint::do_show(
                                  std::ostream &out = std::cout) const {
    out << "RigidBodiesImageFitRestraint. Rigid_bodies: "
       << rigid_bodies_.size() << " Masks: " << std::endl;
    for (unsigned int i=0; i < rigid_bodies_.size(); ++i) {
      out << "rigid body " << rigid_bodies_[i]->get_name() << " Masks "
                                 << maps_[i].size() << std::endl;
    }
  }


unsigned int RigidBodiesImageFitRestraint::get_rigid_body_index(
                                        const core::RigidBody &rb) const {
  IMP_LOG_TERSE( "Check rigid body index " << rb->get_name() << std::endl);
  unsigned int   j = rigid_bodies_.size();
  for ( unsigned int i=0; i < rigid_bodies_.size(); ++i) {
    IMP_LOG_VERBOSE("Comparing " << rigid_bodies_[i].get_particle()
            << " with " << rb.get_particle() << std::endl);

    if(rb.get_particle() == rigid_bodies_[i].get_particle()) {
      j = i;
      break;
    }
  }
  if(j == rigid_bodies_.size() ) {
   IMP_THROW("The rigid body provided is not in the restraint", ValueException);
  }
  return j;
}

unsigned int RigidBodiesImageFitRestraint::get_number_of_masks(
                                    const core::RigidBody &rb) const {
  unsigned int j = get_rigid_body_index(rb);
  return maps_[j].size();

}

IMPEM2D_END_NAMESPACE
