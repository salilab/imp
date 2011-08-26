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
#include <IMP/log.h>

IMPISD_BEGIN_NAMESPACE

using IMP::algebra::internal::TNT::Array1D;
using IMP::algebra::internal::TNT::Array2D;

    GaussianProcessInterpolation::GaussianProcessInterpolation(
                               std::vector<std::vector<double> > x,
                               std::vector<double> sample_mean,
                               std::vector<double> sample_std,
                               std::vector<int> n_obs,
                               UnivariateFunction *mean_function,
                               BivariateFunction *covariance_function) :
        Object("GaussianProcessInterpolation%1%"), x_(x), n_obs_(n_obs),
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
    mean_function_->update();
    compute_m();
    covariance_function_->update();
    compute_W_matrix();
    compute_inverse();
    compute_WSIm();
}

  double GaussianProcessInterpolation::get_posterior_mean(std::vector<double> x)
{
   // std::cerr << "posterior mean at q=" << x[0] << std::endl;
    bool cov = update_covariance();
    bool mean = update_mean();
    if (cov) compute_inverse();
    if (cov || mean) compute_WSIm();
    compute_wx_vector(x);
    double ret=0;
    for (unsigned i=0; i<M_; i++) ret += wx_[i]*WSIm_[i];
    //std::cerr << "wx : ";
    //for (unsigned i=0; i<M_; i++) std::cerr << wx_[i] << " ";
    //std::cerr << std::endl << "WSIm : ";
    //for (unsigned i=0; i<M_; i++) std::cerr << WSIm_[i] << " ";
    //std::cerr << std::endl << "mean func: " << (*mean_function_)(x)[0] 
    //    << std::endl;
    return ret + (*mean_function_)(x)[0];
}

  double GaussianProcessInterpolation::get_posterior_covariance(
          std::vector<double> x1, std::vector<double> x2)
{
    //std::cerr << "posterior covariance at q=" << x1[0] << std::endl;
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
        IMP_LOG(TERSE, "   WSwx["<<i<<"] = "<< right[i] << std::endl);
    }
    if (x1 != x2) compute_wx_vector(x1);
    double ret=0;
    for (unsigned i=0; i<M_; i++)
    {
        ret += wx_[i]*right[i];
    }
    IMP_LOG(TERSE, "   ret = "<<ret<<std::endl);
    return (*covariance_function_)(x1,x2)[0] - ret;
}

  void GaussianProcessInterpolation::compute_I(std::vector<double> mean)
{
    I_ = Array1D<double> (M_);
    IMP_LOG(TERSE, "I: ");
    for (unsigned i=0; i<M_; i++)
    {
        I_[i] = mean[i];
        IMP_LOG(TERSE, I_[i] << " ");
    }
    IMP_LOG(TERSE, std::endl);
}

  void GaussianProcessInterpolation::compute_S(std::vector<double> std,
          std::vector<int> n) 
    { 
        S_ = Array2D<double> (M_,M_,0.0); 
        IMP_LOG(TERSE, "S: ");
        for (unsigned i=0; i<M_; i++) 
        { 
            S_[i][i] = IMP::square(std[i])/double(n[i]); 
            IMP_LOG(TERSE, S_[i][i] << " ");
        } 
    IMP_LOG(TERSE, std::endl);
    }

  void GaussianProcessInterpolation::compute_m()
{
    //std::cerr << "  compute_m" << std::endl;
    m_ = Array1D<double> (M_);
    IMP_LOG(TERSE, "m: ");
    for (unsigned i=0; i<M_; i++)
    {
        m_[i] = (*mean_function_)(x_[i])[0];
        IMP_LOG(TERSE, m_[i] << " ");
    }
    IMP_LOG(TERSE, std::endl);
}

  void GaussianProcessInterpolation::compute_W_matrix()
{
    //std::cerr << "  compute_W_matrix" << std::endl;
    W_ = Array2D<double> (M_,M_);
    for (unsigned i=0; i<M_; i++)
    {
        W_[i][i] = (*covariance_function_)(x_[i],x_[i])[0];
        IMP_LOG(TERSE, "W[" << i << "][" << i << "]: " 
                                << W_[i][i] << std::endl);
        for (unsigned j=i+1; j<M_; j++)
        {
            W_[i][j] = (*covariance_function_)(x_[i],x_[j])[0];
            W_[j][i] = W_[i][j];
            IMP_LOG(TERSE, "W[" << i << "][" << j << "]: " 
                                << W_[i][j] << std::endl);
        }
    }
}

  void GaussianProcessInterpolation::compute_wx_vector(
                                    std::vector<double> xval)
{
    IMP_LOG(TERSE,"  compute_wx_vector at q= " << xval[0] << " ");
    wx_ = Array1D<double> (M_);
    for (unsigned i=0; i<M_; i++)
    {
        wx_[i] = (*covariance_function_)(x_[i],xval)[0];
        IMP_LOG(TERSE, wx_[i] << " ");
    }
    IMP_LOG(TERSE, std::endl);
}


  void GaussianProcessInterpolation::compute_inverse()
{
    IMP_LOG(TERSE,"  compute_inverse" << std::endl);
    //compute W+S
    Array2D<double> WpS;
    WpS = W_.copy();
    for (unsigned i =0; i<M_; i++) WpS[i][i] += S_[i][i];
    IMP_IF_LOG(TERSE) { 
            for (unsigned i=0; i<M_; i++) {
                for (unsigned j=0; j<M_; j++) {
                    IMP_LOG(TERSE, WpS[i][j] << " ");
                }
                IMP_LOG(TERSE, std::endl);
            }
            };
    IMP_LOG(TERSE,"  compute_inverse: LU" << std::endl);
    //compute LU decomp
    LU_.reset(new algebra::internal::JAMA::LU<double> (WpS));
    //get inverse
    IMP_LOG(TERSE,"  compute_inverse: determinant = " <<LU_->det() << std::endl);
    IMP_LOG(TERSE,"  compute_inverse: inverse" << std::endl);
    Array2D<double> id(M_,M_,0.0);
    for (unsigned i=0; i<M_; i++) id[i][i] = 1.0;
    WS_= LU_->solve(id);
    for (unsigned i=0; i<M_; i++) {
        for (unsigned j=0; j<M_; j++) {
            IMP_LOG(TERSE, "WS[" << i << "][" << j << "]: " 
                                << WS_[i][j] << std::endl);
        }
    }
}

 void GaussianProcessInterpolation::compute_WSIm()
{
    WSIm_ = Array1D<double> (M_);
    IMP_LOG(TERSE, "WSIm ");
    for (unsigned i=0; i<M_; i++)
    {
        WSIm_[i] = 0.0;
        for (unsigned j=0; j<M_; j++)
        {
            WSIm_[i] += WS_[i][j] * (I_[j] - m_[j]);
        }
        IMP_LOG(TERSE, WSIm_[i] << " ");
    }
    IMP_LOG(TERSE, std::endl);
    //for (unsigned j=0; j<M_; j++)
    //{
    //    std::cerr << "I-m " << j << " " << I_[j] - m_[j] << std::endl;
    //}
}

  bool GaussianProcessInterpolation::update_mean()
{
    bool ret = mean_function_->has_changed();
    if (ret)
    { 
        IMP_LOG(TERSE, " mean function has changed, updating" << std::endl);
        mean_function_->update();
        compute_m();
    }
    return ret;
}

  bool GaussianProcessInterpolation::update_covariance()
{
    bool ret = covariance_function_->has_changed();
    if (ret)
    {
        IMP_LOG(TERSE, " covariance function has changed, updating" 
                << std::endl);
        covariance_function_->update();
        compute_W_matrix();
    }
    return ret;
}

  void GaussianProcessInterpolation::do_show(std::ostream &out) const
{
    out << "Interpolation via gaussian process" << std::endl;
}

IMPISD_END_NAMESPACE

