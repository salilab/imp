/**
 *  \file ProjectionMask.cpp
 *  \brief projection masks
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/ProjectionMask.h"
#include "IMP/exception.h"

IMPEM2D_BEGIN_NAMESPACE


ProjectionMask::~ProjectionMask() {
  IMP_LOG_VERBOSE(" Projection mask destroyed." << std::endl);
}

ProjectionMask::ProjectionMask(const em::KernelParameters &KP,
         const em::RadiusDependentKernelParameters &params,
         double pixelsize,
         double mass) {
  sq_pixelsize_ = pixelsize*pixelsize;
//  dim_ = 2*floor(params->get_kdist()/pixelsize)+1;
//  data_.create(dim_,dim_,CV_64FC1);
  dim_ = floor(params.get_kdist()/pixelsize);
  int mask_size = 2*dim_+1; // enough to go from -dim to dim
  data_.create(mask_size,mask_size,CV_64FC1);
  data_.setTo(0.0);
  create(KP,params, mass);
}


void ProjectionMask::apply(cv::Mat &m,
                const algebra::Vector2D &v) {
  do_place(data_, m, v);
}





void  ProjectionMask::create(const em::KernelParameters &KP,
                 const em::RadiusDependentKernelParameters &params,
                 double mass) {

  // Decorate the masks to use centered coordinates
  CenteredMat centered_mask(data_);

  IMP_LOG_VERBOSE(" Generating mask.  " << centered_mask);

  double tmp,square_radius;
  for(int i=-dim_;i<=dim_;++i) {
    double isq = static_cast<double>(i*i);
    for(int j=-dim_;j<=dim_;++j) {
      double jsq = static_cast<double>(j*j);
      double ijsq = isq+jsq;
      for(int k=-dim_;k<=dim_;++k) {
        square_radius = (ijsq + static_cast<double>(k*k))*sq_pixelsize_;
        // Add the value to the mask
        tmp= em::EXP(-square_radius * params.get_inv_sigsq());
        // if statement to ensure even sampling within the box
        if (tmp> KP.get_lim() && centered_mask.get_is_in_range(i,j) ) {
          centered_mask(i,j) += params.get_normfac()*tmp * mass;
        }
      }
    }
  }
 IMP_LOG_VERBOSE(" Mask generated.  " << std::endl);
}


  void ProjectionMask::show(std::ostream &out) const {
    out << "ProjectionMask size " << data_.rows << "x" <<  data_.cols
        << std::endl;
  }





ProjectionMaskPtr MasksManager::find_mask(double radius) {
  IMP_LOG_VERBOSE("Finding mask for radius " << radius << std::endl);
  std::map<double,  ProjectionMaskPtr >::iterator iter
                                    = radii2mask_.find(radius);
  if(iter == radii2mask_.end())
    return ProjectionMaskPtr(); // null
  return iter->second;
}


void MasksManager::create_masks(const ParticlesTemp &ps) {
  IMP_LOG_TERSE("Creating Projection Masks " << std::endl);
  ProjectionMaskPtr mask;
  unsigned long n_particles = ps.size();
  for (unsigned long i=0; i<n_particles; i++) {
    core::XYZR xyzr(ps[i]);
    double radius = xyzr.get_radius();
    mask = this->find_mask(radius);
    if (!mask) {
      atom::Mass mass(ps[i]);
      double w = mass.get_mass();
      this->create_mask(radius, w);
    }
  }
  IMP_LOG_TERSE("Finished creating Projection Masks " << std::endl);
}

void MasksManager::create_mask(double radius, double mass) {
  IMP_LOG_VERBOSE("Creating a projection mask for radius " <<
            radius <<std::endl);
  if(is_setup_ == false) {
    IMP_THROW("MasksManager: kernel not setup",ValueException);
  }
  // kernel_params_.set_params(radius); // Due to numerical instability with
                                        // doubles, this call can throw an
                                        // exception
  // This call creates the params, but gives a warning
  const em::RadiusDependentKernelParameters
            &params = kernel_params_.get_params(radius);
  ProjectionMaskPtr ptr(new ProjectionMask(kernel_params_,
                                           params,
                                           pixelsize_,
                                           mass));
  radii2mask_[radius]=ptr;
}

MasksManager::~MasksManager() {
  IMP_LOG_VERBOSE("Masks Manager has been destroyed" << std::endl);
}


void MasksManager::show(std::ostream &out) const {
  out << "MasksManager: " << radii2mask_.size() << " masks. "
      << "Initialized " << is_setup_ << " pixelsize "
      << pixelsize_ << std::endl;

}

IMPEM2D_END_NAMESPACE
