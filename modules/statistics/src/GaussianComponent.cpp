/**
 *  \file GaussianComponent.cpp
 *  \brief A component of a GMM
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/statistics/GaussianComponent.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//using namespace IMP::algebra::internal;
void GaussianComponent::initial_sigma(const std::vector<Array1DD> &data){
 for(int d1=0;d1<dim_;d1++) {
    for(int d2=0;d2<dim_;d2++) {
      sigma_[d1][d2]=0.;
    }
  }
 for(unsigned int i=0;i<data.size();i++) {
    for(int d1=0;d1<dim_;d1++) {
      for(int d2=0;d2<dim_;d2++) {
        sigma_[d1][d2] += (data[i][d1]-mean_[d1])*(data[i][d2]-mean_[d2]);
      }
    }
  }
 for(int d1=0;d1<dim_;d1++) {
   for(int d2=0;d2<dim_;d2++) {
     sigma_[d1][d2] = sigma_[d1][d2]/data.size()+id_mat_[d1][d2];
   }
 }
}

void GaussianComponent::update_sigma(const std::vector<Array1DD> &data,
                                     const Array2DD &data_r) {
 for(int d1=0;d1<dim_;d1++) {
    for(int d2=0;d2<dim_;d2++) {
      sigma_[d1][d2]=0.;
    }
  }
 double norm=0.;
 for(unsigned int i=0;i<data.size();i++) {
    norm += data_r[i][ind_];
    for(int d1=0;d1<dim_;d1++) {
      for(int d2=0;d2<dim_;d2++) {
        sigma_[d1][d2] +=
          data_r[i][ind_]*(data[i][d1]-mean_[d1])*(data[i][d2]-mean_[d2]);
      }
    }
  }
 for(int d1=0;d1<dim_;d1++) {
   for(int d2=0;d2<dim_;d2++) {
     sigma_[d1][d2] = sigma_[d1][d2]/norm+id_mat_[d1][d2]*1e-5f;
   }
 }
}

GaussianComponent::GaussianComponent(const Array1DD &m,const Array2DD &s,
                                     double prior, int index,
                                     std::string name): Object(name) {
  init_component(m,s,prior,index);
}
GaussianComponent::GaussianComponent(const IMP::algebra::Ellipsoid3D &e,
                                     float prior,int index,
                                     std::string name): Object(name) {
  IMP::algebra::Vector3D center = e.get_center();
  Array1DD mean(3);
  for (int i=0;i<3;i++) {
    mean[i]=center[i];
  }
  Array2DD sigma(3,3);
  sigma[0][0]=e.get_radius(0);
  sigma[1][1]=e.get_radius(1);
  sigma[2][2]=e.get_radius(2);
  init_component(mean,sigma,prior,index);
}

void GaussianComponent::init_component(const Array1DD &m,const Array2DD &s,
                                       double prior, int index) {
  IMP_LOG(VERBOSE,"start setting a gaussian component"<< std::endl);
  ind_ = index;
  dim_ = m.dim1();
  //set the mean and covariance
  sigma_ = Array2DD(dim_,dim_);
  id_mat_ = Array2DD(dim_,dim_,0.);
  for(int i=0;i<dim_;i++) id_mat_[i][i]=1.;
  mean_ = Array1DD(dim_);
  IMP_INTERNAL_CHECK(s.dim1() == dim_,
                     "The input sigma is not of the right size");
  IMP_INTERNAL_CHECK(s.dim2() == dim_,
                     "The input sigma is not of the right size");
  IMP_INTERNAL_CHECK(m.dim() == dim_,
                     "The input mean is not of the right size");
  for(int i=0;i<dim_;i++) {
    IMP_LOG(VERBOSE,"i: "<< i << " :: " << mean_.dim() << std::endl);
    mean_[i]= m[i];// + IMP::statistics::random_uniform(-10,10);//TODO - change
    for(int j=0;j<dim_;j++) {
      IMP_LOG(VERBOSE,"j: "<< j << " :: " << sigma_.dim1()
              << " " << sigma_.dim2() << std::endl);
      IMP_LOG(VERBOSE,"id_mat: "<< j << " :: " << id_mat_.dim1()
              << " " << id_mat_.dim2() << std::endl);
      IMP_LOG(VERBOSE,"s: "<< j << " :: " << s.dim1()
              << " " << s.dim2() << std::endl);
      sigma_[i][j]=s[i][j] + id_mat_[i][j]* 1e-5f;
    }
  }
  IMP_LOG(VERBOSE,"==5"<<std::endl);
  //  initial_sigma(data);
  //  update_sigma(data,data_r);
  prior_ = prior;
  IMP_LOG(VERBOSE,"going to update parameters"<< std::endl);
  update_parameters();
  IMP_LOG(VERBOSE,"finish setting gaussian components"<< std::endl);
}

void GaussianComponent::update_parameters() {
  sigma_inv_ = algebra::internal::TNT::inverse(sigma_);
  sigma_det_ = algebra::internal::TNT::determinant(sigma_);
  mult_factor_ = 1./(sqrt(pow(2*IMP::PI,dim_)*std::abs(sigma_det_)));
}

void GaussianComponent::show(std::ostream &out) const {
  out << "component : " << ind_ << " dim: "<< dim_
      << " prior: "<< prior_ <<std::endl;
  out << "component : " << ind_ << " mean: \t";
  for(int i=0;i<mean_.dim1();i++) out << mean_[i] <<" ";
  out<<std::endl;
  out << "component : " << ind_ << " sigma: ";
  for(int i=0;i<sigma_.dim1();i++) {
    out<<"\t";
    for(int j=0;j<sigma_.dim2();j++) {
      out<<sigma_[i][j] <<" ";
    }
    out << std::endl;
  }
  out << "sigma_inv: ";
  for(int i=0;i<sigma_inv_.dim1();i++) {
    out<<"\t";
    for(int j=0;j<sigma_inv_.dim2();j++) out<<sigma_inv_[i][j] <<" ";
    out << std::endl;
  }
  out << "sigma_det: \t" << sigma_det_ << std::endl;
  out << "mult_factor: \t" << mult_factor_ << std::endl;
}



void GaussianComponent::show_single_line(std::ostream &out) const {
  out << "component : " << ind_ << " dim: "<< dim_ << " prior: "<< prior_;
  out << "mean: ";
  for(int i=0;i<mean_.dim1();i++) out << mean_[i] <<" ";
  out << " sigma: ";
  for(int i=0;i<sigma_.dim1();i++) {
    for(int j=0;j<sigma_.dim2();j++) {
      out<<sigma_[i][j] <<" ";
    }
  }
  out << "sigma_inv: ";
  for(int i=0;i<sigma_inv_.dim1();i++) {
    for(int j=0;j<sigma_inv_.dim2();j++) out<<sigma_inv_[i][j] <<" ";
  }
  out << " sigma_det: " << sigma_det_ << " mult_factor: " << mult_factor_;
  out << std::endl;
}


double GaussianComponent::pdf(const Array1DD &x) const {
  Array1DD diff = algebra::internal::TNT::subtract(x,mean_);
  Array1DD diff_v = Array1DD(mean_.dim1());
  double aa = algebra::internal::TNT::dot_product(
                 diff,algebra::internal::TNT::multiply(sigma_inv_,diff));
  return mult_factor_*std::exp(-0.5*aa);
  /*
  for(int j=0;j<mean_.dim1();j++) {
    double v=0.;
    for(int i=0;i<mean_.dim1();i++) {
      v += diff[i]*sigma_inv_[j][i];
    }
    diff_v[j]=v;
  }
  double ans = mult_factor_*std::exp(-0.5*
         algebra::internal::TNT::dot_product(diff_v,diff));
  return ans;
  */
}

void GaussianComponent::update(const std::vector<Array1DD> &data,
                               const Array2DD &data_r) {
  double sum_r = 0.; //sum over the responsibilities of the data points
  Array1DD sum_wd(dim_,0.); // sum weighted data
  for(int i=0;i<data_r.dim1();i++) {
    sum_r += data_r[i][ind_];
    sum_wd = algebra::internal::TNT::add(sum_wd,
                   algebra::internal::TNT::multiply(data_r[i][ind_],data[i]));
  }
  IMP_INTERNAL_CHECK(not IMP::algebra::get_are_almost_equal(sum_r,0.,GMM_EPS),
      "no data points were assigned to Gaussian component:"
                  << ind_ << std::endl);

//  for(int i=0;i<data_r.dim1();i++) {
//    std::cout<<data_r[i][ind_]<<"  ";
//  }
//  std::cout<<std::endl<<"sum_r: " << sum_r<<std::endl;
  prior_ = sum_r/data.size();
  mean_ = algebra::internal::TNT::multiply(1./sum_r,sum_wd);
  update_sigma(data,data_r);
//   double val = 0.;
//   Array1DD diff;
//   for(int i=0;i<data_r.dim1();i++) {
//     diff = algebra::internal::TNT::subtract(data[i],mean_);
//     val += data_r[i][ind_] * algebra::internal::TNT::dot_product(diff,diff);
//   }
//   val = val / sum_r;
//   for(int i=0;i<sigma_.dim1();i++) {
//     for(int j=0;j<sigma_.dim2();j++) {
//       sigma_[i][j]=val;
//     }
//   }
//   sigma_ = sigma_ + id_mat_;//to make the matrix inversable
  update_parameters();
}

//! Transform a 3D Gaussian to an ellipdoid
IMP::algebra::Ellipsoid3D get_ellipsoid3D(const GaussianComponent &gc) {
  IMP_INTERNAL_CHECK(gc.get_dim()==3,"The Gaussian is not of 3D");
  Array1DD mean=gc.get_mean();
  Array2DD sigma=gc.get_sigma();
  return IMP::algebra::Ellipsoid3D(
        IMP::algebra::Vector3D(mean[0],mean[1],mean[2]),
        sigma[0][0],sigma[1][1],sigma[2][2],
        IMP::algebra::get_identity_rotation_3d());
}

IMP::algebra::Sphere3D get_sphere3D(const GaussianComponent &gc) {
  IMP_INTERNAL_CHECK(gc.get_dim()==3,"The Gaussian is not of 3D");
  Array1DD mean=gc.get_mean();
  return IMP::algebra::Sphere3D(
            IMP::algebra::Vector3D(mean[0],mean[1],mean[2]),5.);
}

IMPSTATISTICS_END_NAMESPACE
