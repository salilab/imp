/**
 *  \file ProjectionMask.cpp
 *  \brief projection masks
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
**/

#include "IMP/em2D/ProjectionMask.h"

IMPEM2D_BEGIN_NAMESPACE

ProjectionMask::ProjectionMask(const em::KernelParameters &KP,
         const em::RadiusDependentKernelParameters *params,double voxelsize) {
  sq_voxsize_ = voxelsize*voxelsize;
  dim_ = 2*floor(params->get_kdist()/voxelsize)+1;
  this->resize(dim_,dim_);
  this->centered_start();
  this->generate(KP,params);
}

void  ProjectionMask::generate(const em::KernelParameters &KP,
                 const em::RadiusDependentKernelParameters *params) {
  double tmp,square_radius;
  for(int i=-dim_;i<=dim_;++i) {
    double isq = (double)i*i;
    for(int j=-dim_;j<=dim_;++j) {
      double jsq = (double)j*j;
      for(int k=-dim_;k<=dim_;++k) {
        square_radius = (isq+jsq+(double)k*k)*sq_voxsize_;
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

void ProjectionMask::apply(algebra::Matrix2D_d &m,
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

void MasksManager::create_mask(double radius) {
  IMP_LOG(IMP::VERBOSE,"Creating a projection mask for radius " <<
            radius <<std::endl);

  const  em::RadiusDependentKernelParameters *params;
  kernel_params_.set_params(radius);
  params = kernel_params_.get_params(radius);
  // Create the mask
  radii2mask_[radius]=new ProjectionMask(kernel_params_,params,pixelsize_);
}

ProjectionMask* MasksManager::find_mask(double radius) {
  std::map<double, ProjectionMask *>::iterator iter
      = radii2mask_.find(radius);
  if(iter == radii2mask_.end()) {
    return NULL;
  } else {
    return iter->second;
  }
}

void MasksManager::generate_masks(const Particles &ps) {
  IMP_LOG(IMP::TERSE,"Genereating ProjectionMasks " << std::endl);
  ProjectionMask *mask;
  FloatKey radius_key = core::XYZR::get_default_radius_key();
  unsigned long n_particles = ps.size();
  for (unsigned long ii=0; ii<n_particles; ii++) {
    core::XYZR xyzr=core::XYZR(ps[ii],radius_key);
    double radius = xyzr.get_radius();
    mask = this->find_mask(radius);
    if (!mask) this->create_mask(radius);
  }
}




IMPEM2D_END_NAMESPACE
