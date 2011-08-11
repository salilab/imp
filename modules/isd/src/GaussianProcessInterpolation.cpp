/**
 *  \file GaussianProcessInterpolation.cpp  
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/GaussianProcessInterpolation.h>
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/jama_lu.h>
#include <boost/scoped_ptr.hpp>

IMPISD_BEGIN_NAMESPACE

using IMP::algebra::internal::TNT::Array1D;
using IMP::algebra::internal::TNT::Array2D;

    GaussianProcessInterpolation::GaussianProcessInterpolation(
                               std::vector<std::vector<double> > x,
                               std::vector<double> sample_mean,
                               std::vector<double> sample_std,
                               std::vector<unsigned> n_obs,
                               UnivariateFunction *mean_function,
                               BivariateFunction *covariance_function) :
        Object("GaussianProcessInterpolation%1%"), x_(x),
        mean_function_(mean_function), 
        covariance_function_(covariance_function) 
{
    //store dimensions
    M_ = x.size();
    N_ = x[0].size();
    //basic checks
    IMP_USAGE_CHECK(sample_mean.size() == M_, 
            "sample_mean should have the same size as x");
    IMP_USAGE_CHECK(sample_std.size() == M_, 
            "sample_std should have the same size as x");
    IMP_USAGE_CHECK(n_obs.size() == M_, 
            "n_obs should have the same size as x");
    IMP_USAGE_CHECK(mean_function->get_ndims_x() == N_, 
            "mean_function should have " << N_ << " input dimensions");
    IMP_USAGE_CHECK(mean_function->get_ndims_y() == 1,
            "mean_function should have 1 output dimension");
    IMP_USAGE_CHECK(covariance_function->get_ndims_x1() == N_, 
            "covariance_function should have " << N_ 
                        << " input dimensions for first vector");
    IMP_USAGE_CHECK(covariance_function->get_ndims_x2() == N_, 
            "covariance_function should have " << N_ 
                        << " input dimensions for second vector");
    IMP_USAGE_CHECK(covariance_function->get_ndims_y() == 1,
            "covariance_function should have 1 output dimension");
    //compute needed matrices
    compute_I(sample_mean);
    compute_S(sample_std,n_obs);
    mean_function_->update_if_changed();
    compute_m();
    covariance_function_->update_if_changed();
    compute_W_matrix();
    compute_inverse();
    compute_WSIm();
}

  double GaussianProcessInterpolation::get_posterior_mean(std::vector<double> x)
{
    if (update_covariance()) compute_inverse();
    if (update_mean()) compute_WSIm();
    compute_wx_vector(x);
    double ret=0;
    for (unsigned i=0; i<M_; i++) ret += wx_[i]*WSIm_[i];
    return ret + (*mean_function_)(x)[0];
}

  double GaussianProcessInterpolation::get_posterior_covariance(
          std::vector<double> x1, std::vector<double> x2)
{
    if (update_covariance()) compute_inverse();
    compute_wx_vector(x2);
    Array1D<double> right(M_);
    for (unsigned i=0; i<M_; i++)
    {
        right[i] = 0.0;
        for (unsigned j=0; j<M_; j++)
        {
            right[i] += WS_[i][j]*wx_[j];
        }
    }
    if (x1 != x2) compute_wx_vector(x1);
    double ret=0;
    for (unsigned i=0; i<M_; i++)
    {
        ret += wx_[i]*right[i];
    }
    return (*covariance_function_)(x1,x2)[0] - ret;
}

  void GaussianProcessInterpolation::compute_I(std::vector<double> mean)
{
    I_ = Array1D<double> (M_);
    for (unsigned i=0; i<M_; i++)
    {
        I_[i] = mean[i];
    }
}

  void GaussianProcessInterpolation::compute_S(std::vector<double> std,
          std::vector<unsigned> n) 
    { 
        S_ = Array2D<double> (M_,M_,0.0); 
        for (unsigned i=0; i<M_; i++) 
        { 
            S_[i][i] = IMP::square(std[i])/double(n[i]); 
        } 
    }

  void GaussianProcessInterpolation::compute_m()
{
    m_ = Array1D<double> (M_);
    for (unsigned i=0; i<M_; i++)
    {
        m_[i] = (*mean_function_)(x_[i])[0];
    }
}

  void GaussianProcessInterpolation::compute_W_matrix()
{
    W_ = Array2D<double> (M_,M_);
    for (unsigned i=0; i<M_; i++)
    {
        W_[i][i] = (*covariance_function_)(x_[i],x_[i])[0];
        for (unsigned j=i+1; j<M_; j++)
        {
            W_[i][j] = (*covariance_function_)(x_[i],x_[j])[0];
            W_[j][i] = W_[i][j];
        }
    }
}

  void GaussianProcessInterpolation::compute_wx_vector(
                                    std::vector<double> xval)
{
    wx_ = Array1D<double> (M_);
    for (unsigned i=0; i<M_; i++)
    {
        wx_[i] = (*covariance_function_)(x_[i],xval)[0];
    }
}


  void GaussianProcessInterpolation::compute_inverse()
{
    //compute W+S
    Array2D<double> WpS;
    WpS = W_.copy();
    for (unsigned i =0; i<M_; i++) WpS[i][i] += S_[i][i];
    //compute LU decomp
    LU_.reset(new algebra::internal::JAMA::LU<double> (WpS));
    //get inverse
    Array2D<double> id(M_,M_,0.0);
    for (unsigned i=0; i<M_; i++) id[i][i] = 1.0;
    WS_= LU_->solve(id);
}

 void GaussianProcessInterpolation::compute_WSIm()
{
    WSIm_ = Array1D<double> (M_);
    for (unsigned i=0; i<M_; i++)
    {
        WSIm_[i] = 0.0;
        for (unsigned j=0; j<M_; j++)
        {
            WSIm_[i] += WS_[i][j] * (I_[j] - m_[j]);
        }
    }
}

  bool GaussianProcessInterpolation::update_mean()
{
    bool ret = mean_function_->update_if_changed();
    if (ret) compute_m();
    return ret;
}

  bool GaussianProcessInterpolation::update_covariance()
{
    bool ret = covariance_function_->update_if_changed();
    if (ret) compute_W_matrix();
    return ret;
}

  void GaussianProcessInterpolation::do_show(std::ostream &out) const
{
    out << "Interpolation via gaussian process" << std::endl;
}

IMPISD_END_NAMESPACE

