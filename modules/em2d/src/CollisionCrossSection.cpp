/**
 *  \file CollisionCrossSection
 *  \brief Routines for Ion Mobility Mass Spectrometry
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */


#include "IMP/em2d/CollisionCrossSection.h"
#include "IMP/exception.h"
#include "IMP/atom/Mass.h"


IMPEM2D_BEGIN_NAMESPACE


CollisionCrossSection::CollisionCrossSection(unsigned int  n_projections,
                        double resolution,
                        double pixel_size,
                        double projection_image_size
                                             ):
                        Object("CollisionCrossSection%1%"),
                        n_projections_(n_projections),
                        resolution_(resolution),
                        pixel_size_(pixel_size),
                        img_size_(projection_image_size),
                        particles_set_(false) {
  regs_ = em2d::get_evenly_distributed_registration_results(n_projections_);
  average_projection_.create(img_size_, img_size_, CV_64FC1);
  masks_manager_ = MasksManagerPtr(new MasksManager);
  masks_manager_->setup_kernel(resolution_,pixel_size_);
}


double CollisionCrossSection::get_ccs() const {
  if(particles_set_) {
    return collision_cross_section_;
  } else {
    IMP_THROW("CollisionCrossSection: Can't recover the value, "
              "the model particles are not set." , ValueException);
  }
}

void CollisionCrossSection::set_model_particles(const ParticlesTemp &ps) {

  IMP_LOG_TERSE("CollisionCrossSection: Model particles set" << std::endl);

  for (unsigned int i = 0; i < ps.size() ; ++i) {
    IMP_USAGE_CHECK((core::XYZR::particle_is_instance(ps[i]) &&
                     atom::Mass::particle_is_instance(ps[i])),
       "Particle " << i
       << " does not have the required attributes" << std::endl);
  }
  masks_manager_->create_masks(ps);
  // Compute projections
  collision_cross_section_ = 0.0;
  for (unsigned int i=0; i < n_projections_; ++i) {
    ProjectingOptions options(pixel_size_, resolution_);
    do_project_particles(ps,
                         average_projection_,
                         regs_[i].get_rotation(),
                         pixel_size_*regs_[i].get_shift_3d(),
                         options,
                         masks_manager_);
    collision_cross_section_ += get_projected_area(average_projection_);
  }
  collision_cross_section_ /= static_cast<double>(n_projections_);
  particles_set_ = true;
}

 double CollisionCrossSection::get_projected_area(const cv::Mat &m) const {
  // compute the ccs
  unsigned int pixels_above_zero = 0;
  for (int i=0; i < m.rows; ++i) {
    for (int j=0; j < m.cols; ++j) {
      if (m.at<double>(i, j) > 0) pixels_above_zero++;
    }
  }
  return pixels_above_zero * pixel_size_ * pixel_size_;
}



IMPEM2D_END_NAMESPACE
