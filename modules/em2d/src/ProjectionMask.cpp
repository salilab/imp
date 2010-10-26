/**
 *  \file ProjectionMask.cpp
 *  \brief projection masks
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
**/

#include "IMP/em2d/ProjectionMask.h"
#include "IMP/exception.h"

IMPEM2D_BEGIN_NAMESPACE

OldProjectionMask::OldProjectionMask(const em::KernelParameters &KP,
         const em::RadiusDependentKernelParameters *params,double pixelsize) {
  sq_pixelsize_ = pixelsize*pixelsize;
  dim_ = 2*floor(params->get_kdist()/pixelsize)+1;
  this->resize(dim_,dim_);
  this->centered_start();
  this->generate(KP,params);
}


Projection_Mask::Projection_Mask(const em::KernelParameters &KP,
         const em::RadiusDependentKernelParameters *params,double pixelsize) {
  sq_pixelsize_ = pixelsize*pixelsize;
  dim_ = 2*floor(params->get_kdist()/pixelsize)+1;
  this->create(dim_,dim_,CV_64FC1);
  this->setTo(0.0);
  this->generate(KP,params);
}

void  OldProjectionMask::generate(const em::KernelParameters &KP,
                 const em::RadiusDependentKernelParameters *params) {
  double tmp,square_radius;
  for(int i=-dim_;i<=dim_;++i) {
    double isq = (double)i*i;
    for(int j=-dim_;j<=dim_;++j) {
      double jsq = (double)j*j;
      for(int k=-dim_;k<=dim_;++k) {
        square_radius = (isq+jsq+(double)k*k)*sq_pixelsize_;
        // Add the value to the mask
        tmp= em::EXP(-square_radius * params->get_inv_sigsq());
        // if statement to ensure even sampling within the box
        if (tmp> KP.get_lim() &&
            i>=this->get_start(0) && i<=this->get_finish(0) &&
            j>=this->get_start(1) && j<=this->get_finish(1) ) {
          (*this)(i,j) += params->get_normfac()*tmp;
        }
      }
    }
  }
}


void  Projection_Mask::generate(const em::KernelParameters &KP,
                 const em::RadiusDependentKernelParameters *params) {

  // Decorate the masks to use centered coordinates
  CenteredMat centered_mask(*this);

  IMP_LOG(IMP::VERBOSE," Generating centered of size : "
                                          << dim_ << " " << centered_mask);

  double tmp,square_radius;
  for(int i=-dim_;i<=dim_;++i) {
    double isq = (double)i*i;
    for(int j=-dim_;j<=dim_;++j) {
      double jsq = (double)j*j;
      for(int k=-dim_;k<=dim_;++k) {
        square_radius = (isq+jsq+(double)k*k)*sq_pixelsize_;
        // Add the value to the mask
        tmp= em::EXP(-square_radius * params->get_inv_sigsq());
        // if statement to ensure even sampling within the box
        if (tmp> KP.get_lim() && centered_mask.get_is_in_range(i,j) ) {
          centered_mask(i,j) += params->get_normfac()*tmp;
        }
      }
    }
  }
  IMP_LOG(IMP::VERBOSE, "Generated mask : " << std::endl );
  //show(*this);
}


void OldProjectionMask::apply(algebra::Matrix2D_d &m,
           const algebra::Vector2D &v,double weight) {
  int idx[2];
  int vi= algebra::get_rounded(v[0]);
  int vj= algebra::get_rounded(v[1]);

  for(int i=this->get_start(0);i<=this->get_finish(0);++i) {
    for(int j=this->get_start(1);j<=this->get_finish(1);++j) {
      // No interpolation is done, just round the values to integers
      idx[0]=i+vi; idx[1]=j+vj;
      if(m.is_logical_element(idx)) m(idx) += (*this)(i,j)*weight;
    }
  }
}


void Projection_Mask::apply(cv::Mat &m,
                const algebra::Vector2D &v,double weight) {
  int vi= algebra::get_rounded(v[0]);
  int vj= algebra::get_rounded(v[1]);
  //cv::Mat &mat = *this; // get a reference of class cv::Mat from this
  CenteredMat centered_mask(*this); // Now pass to CenteredMat
  CenteredMat centered_m(m);

//  IMP_LOG(IMP::VERBOSE,"APPLYING MASK: " << centered_mask << " TO " <<
//          centered_m << std::endl);

  for(int i=centered_mask.get_start(0);i <= centered_mask.get_end(0);++i) {
    for(int j=centered_mask.get_start(1);j <= centered_mask.get_end(1);++j) {
      // No interpolation is done, just round the values to integers
      int row=i+vi;
      int col=j+vj;
      if(centered_m.get_is_in_range(row,col)) {
        centered_m(row,col) += centered_mask(i,j)*weight;
      }
    }
  }
}

void OldMasksManager::create_mask(double radius) {
  IMP_LOG(IMP::VERBOSE,"Creating a projection mask for radius " <<
            radius <<std::endl);
  if(is_initialized_ == false) {
    IMP_THROW("OldMasksManager: kernel not initialized",ValueException);
  }
  const  em::RadiusDependentKernelParameters *params;
  kernel_params_.set_params(radius);
  params = kernel_params_.get_params(radius);
  // Create the mask
  radii2mask_[radius]=new OldProjectionMask(kernel_params_,params,pixelsize_);
}

void Masks_Manager::create_mask(double radius) {
  IMP_LOG(IMP::VERBOSE,"Creating a projection mask for radius " <<
            radius <<std::endl);
  if(is_initialized_ == false) {
    IMP_THROW("Masks_Manager: kernel not initialized",ValueException);
  }
  const  em::RadiusDependentKernelParameters *params;
  kernel_params_.set_params(radius);
  params = kernel_params_.get_params(radius);
  // Create the mask
  radii2mask_[radius]=new Projection_Mask(kernel_params_,params,pixelsize_);
}

OldProjectionMask* OldMasksManager::find_mask(double radius) {
  std::map<double, OldProjectionMask *>::iterator iter
      = radii2mask_.find(radius);
  if(iter == radii2mask_.end()) {
    return NULL;
  } else {
    return iter->second;
  }
}

Projection_Mask* Masks_Manager::find_mask(double radius) {
  std::map<double, Projection_Mask *>::iterator iter
                                    = radii2mask_.find(radius);
  IMP_LOG(IMP::VERBOSE,"trying to find mask " << std::endl);
  if(iter == radii2mask_.end()) {
    return NULL;
  } else {
    return iter->second;
  }
}

void OldMasksManager::generate_masks(const ParticlesTemp &ps) {
  IMP_LOG(IMP::TERSE,"Genereating OldProjectionMasks " << std::endl);
  OldProjectionMask *mask;
  unsigned long n_particles = ps.size();
  for (unsigned long i=0; i<n_particles; i++) {
    core::XYZR xyzr(ps[i]);
    double radius = xyzr.get_radius();
    mask = this->find_mask(radius);
    if (!mask) this->create_mask(radius);
  }
}

void Masks_Manager::generate_masks(const ParticlesTemp &ps) {
  IMP_LOG(IMP::TERSE,"Generating Projection Masks " << std::endl);
  Projection_Mask *mask;
  unsigned long n_particles = ps.size();
  for (unsigned long i=0; i<n_particles; i++) {
    core::XYZR xyzr(ps[i]);
    double radius = xyzr.get_radius();
    mask = this->find_mask(radius);
    if (!mask) this->create_mask(radius);
  }
}




IMPEM2D_END_NAMESPACE
