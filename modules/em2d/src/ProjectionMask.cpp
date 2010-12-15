/**
 *  \file ProjectionMask.cpp
 *  \brief projection masks
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/ProjectionMask.h"
#include "IMP/exception.h"

IMPEM2D_BEGIN_NAMESPACE


ProjectionMask::~ProjectionMask() {
  IMP_LOG(IMP::VERBOSE," Projection mask destroyed." << std::endl);
}

ProjectionMask::ProjectionMask(const em::KernelParameters &KP,
         const em::RadiusDependentKernelParameters *params,double pixelsize) {
  sq_pixelsize_ = pixelsize*pixelsize;
  dim_ = 2*floor(params->get_kdist()/pixelsize)+1;
  data_.create(dim_,dim_,CV_64FC1);
  data_.setTo(0.0);
  generate(KP,params);
}


void  ProjectionMask::generate(const em::KernelParameters &KP,
                 const em::RadiusDependentKernelParameters *params) {

  // Decorate the masks to use centered coordinates
  CenteredMat centered_mask(data_);

  IMP_LOG(IMP::VERBOSE," Generating mask.  " << centered_mask);

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
}


void ProjectionMask::apply(cv::Mat &m,
                const algebra::Vector2D &v,double weight) {
  int vi= algebra::get_rounded(v[0]);
  int vj= algebra::get_rounded(v[1]);
  CenteredMat centered_mask(data_); // Now pass to CenteredMat
  CenteredMat centered_m(m);
/**
  IMP_LOG(IMP::VERBOSE,"ProjectionMask::apply mask " << this->rows << "x"
               << this->cols << " "           << centered_mask
    << " vi " << vi << " vj " << vj << "Mat m " << m.rows
    << "x" << m.cols << std::endl);
  std::cout << "fast: ProjectionMask::apply mask " << this->rows << "x"
           << this->cols << " "           << centered_mask
          << " vi " << vi << " vj " << vj << "Mat m "
            << m.rows << "x" << m.cols << std::endl;
**/
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




  void ProjectionMask::show(std::ostream &out) const {
    out << "ProjectionMask size " << data_.rows << "x" <<  data_.cols
        << std::endl;
  }





ProjectionMaskPtr MasksManager::find_mask(double radius) {
  std::map<double,  ProjectionMaskPtr >::iterator iter
                                    = radii2mask_.find(radius);
  //IMP_LOG(IMP::VERBOSE,"trying to find mask " << std::endl);
  if(iter == radii2mask_.end()) {
    return ProjectionMaskPtr(); // null
  } else {
    return iter->second;
  }
}


void MasksManager::generate_masks(const ParticlesTemp &ps) {
  IMP_LOG(IMP::TERSE,"Generating Projection Masks " << std::endl);
  ProjectionMaskPtr mask;
  unsigned long n_particles = ps.size();
  for (unsigned long i=0; i<n_particles; i++) {
    core::XYZR xyzr(ps[i]);
    double radius = xyzr.get_radius();
    mask = this->find_mask(radius);
    if (!mask) this->create_mask(radius);
  }
}

void MasksManager::create_mask(double radius) {
  IMP_LOG(IMP::VERBOSE,"Creating a projection mask for radius " <<
            radius <<std::endl);
  if(is_initialized_ == false) {
    IMP_THROW("MasksManager: kernel not initialized",ValueException);
  }
  const  em::RadiusDependentKernelParameters *params;
  kernel_params_.set_params(radius);
  params = kernel_params_.get_params(radius);
  ProjectionMaskPtr ptr(new ProjectionMask(kernel_params_,params,pixelsize_));
  radii2mask_[radius]=ptr;
}

MasksManager::~MasksManager() {
  IMP_LOG(IMP::VERBOSE,"Masks Manager has been destroyed" << std::endl);
//  std::cout << "Masks Manager has been destroyed" << std::endl;
}


void MasksManager::show(std::ostream &out) const {
  out << "MasksManager: " << radii2mask_.size() << " masks. "
      << "Initialized " << is_initialized_ << " pixelsize "
      << pixelsize_ << std::endl;

}

IMPEM2D_END_NAMESPACE
