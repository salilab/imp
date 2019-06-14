/**
 *  \file ProjectionMask.cpp
 *  \brief projection masks
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/ProjectionMask.h"
#include "IMP/exception.h"

IMPEM2D_BEGIN_NAMESPACE

ProjectionMask::~ProjectionMask() {
  IMP_LOG_VERBOSE(" Projection mask destroyed." << std::endl);
}

ProjectionMask::ProjectionMask(const em::KernelParameters& kparams,
                               double pixelsize,
                               double mass) {
  sq_pixelsize_ = pixelsize * pixelsize;
  dim_ = floor(kparams.get_rkdist() / pixelsize);
  int mask_size = 2 * dim_ + 1;  // enough to go from -dim to dim
  data_.create(mask_size, mask_size, CV_64FC1);
  data_.setTo(0.0);
  create(kparams, mass);
}

void ProjectionMask::apply(cv::Mat &m, const algebra::Vector2D &v) {
  do_place(data_, m, v);
}

void ProjectionMask::create(const em::KernelParameters &kparams,
                            double mass) {

  // Decorate the masks to use centered coordinates
  CenteredMat centered_mask(data_);

  IMP_LOG_VERBOSE(" Generating mask.  " << centered_mask);

  double tmp, square_radius;
  for (int i = -dim_; i <= dim_; ++i) {
    double isq = static_cast<double>(i * i);
    for (int j = -dim_; j <= dim_; ++j) {
      double jsq = static_cast<double>(j * j);
      double ijsq = isq + jsq;
      for (int k = -dim_; k <= dim_; ++k) {
        square_radius = (ijsq + static_cast<double>(k * k)) * sq_pixelsize_;
        // Add the value to the mask
        tmp = em::EXP(-square_radius * kparams.get_inv_rsigsq());
        // if statement to ensure even sampling within the box
        if (tmp > kparams.get_lim() && centered_mask.get_is_in_range(i, j)) {
          centered_mask(i, j) += kparams.get_rnormfac() * tmp * mass;
        }
      }
    }
  }
  IMP_LOG_VERBOSE(" Mask generated.  " << std::endl);
}

void ProjectionMask::show(std::ostream &out) const {
  out << "ProjectionMask size " << data_.rows << "x" << data_.cols << std::endl;
}

ProjectionMaskPtr MasksManager::find_mask(double mass) {
  IMP_LOG_VERBOSE("Finding mask for mass " << mass << std::endl);
  std::map<double, ProjectionMaskPtr>::iterator iter = mass2mask_.find(mass);
  if (iter == mass2mask_.end()) return ProjectionMaskPtr();  // null
  return iter->second;
}

void MasksManager::create_masks(const ParticlesTemp &ps) {
  IMP_LOG_TERSE("Creating Projection Masks " << std::endl);
  ProjectionMaskPtr mask;
  unsigned long n_particles = ps.size();
  for (unsigned long i = 0; i < n_particles; i++) {
    double w = atom::Mass(ps[i]).get_mass();
    mask = this->find_mask(w);
    if (!mask) {
      this->create_mask(w);
    }
  }
  IMP_LOG_TERSE("Finished creating Projection Masks " << std::endl);
}

void MasksManager::create_mask(double mass) {
  IMP_LOG_VERBOSE("Creating a projection mask for mass " << mass
                  << std::endl);
  if (is_setup_ == false) {
    IMP_THROW("MasksManager: kernel not setup", ValueException);
  }
  ProjectionMaskPtr ptr(
      new ProjectionMask(kernel_params_, pixelsize_, mass));
  mass2mask_[mass] = ptr;
}

MasksManager::~MasksManager() {
  IMP_LOG_VERBOSE("Masks Manager has been destroyed" << std::endl);
}

void MasksManager::show(std::ostream &out) const {
  out << "MasksManager: " << mass2mask_.size() << " masks. "
      << "Initialized " << is_setup_ << " pixelsize " << pixelsize_
      << std::endl;
}

IMPEM2D_END_NAMESPACE
