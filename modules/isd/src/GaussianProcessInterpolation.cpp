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
                               FloatsList x,
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
    // set all flags to false = need update.
    force_mean_update();
    force_covariance_update();
    //compute needed matrices
    compute_I(sample_mean);
    compute_S(sample_std,n_obs);
}

  void GaussianProcessInterpolation::force_mean_update()
{
    flag_m_ = false;
    flag_WSIm_ = false;
}

  void GaussianProcessInterpolation::force_covariance_update()
{
    flag_WS_ = false;
    flag_WSIm_ = false;
    flag_W_ = false;
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

  double GaussianProcessInterpolation::get_posterior_mean(std::vector<double> x)
{
   // std::cerr << "posterior mean at q=" << x[0] << std::endl;
    Array1D<double> wx(get_wx_vector(x));
    Array1D<double> WSIm(get_WSIm());
    double ret=0;
    for (unsigned i=0; i<M_; i++) ret += wx[i]*WSIm[i];
    //std::cerr << "wx : ";
    //for (unsigned i=0; i<M_; i++) std::cerr << wx[i] << " ";
    //std::cerr << std::endl << "WSIm : ";
    //for (unsigned i=0; i<M_; i++) std::cerr << WSIm[i] << " ";
    //std::cerr << std::endl << "mean func: " << (*mean_function_)(x)[0] 
    //    << std::endl;
    return ret + (*mean_function_)(x)[0]; //licit because WSIm is up to date
}

  double GaussianProcessInterpolation::get_posterior_covariance(
          std::vector<double> x1, std::vector<double> x2)
{
    //std::cerr << "posterior covariance at q=" << x1[0] << std::endl;
    Array1D<double> wx(get_wx_vector(x2));
    Array2D<double> WS(get_WS());
    Array1D<double> right(M_);
    for (unsigned i=0; i<M_; i++)
    {
        right[i] = 0.0;
        for (unsigned j=0; j<M_; j++)
        {
            right[i] += WS[i][j]*wx[j];
        }
        IMP_LOG(TERSE, "   WSwx["<<i<<"] = "<< right[i] << std::endl);
    }
    if (x1 != x2) wx = get_wx_vector(x1);
    double ret=0;
    for (unsigned i=0; i<M_; i++)
    {
        ret += wx[i]*right[i];
    }
    IMP_LOG(TERSE, "   ret = "<<ret<<std::endl);
    return (*covariance_function_)(x1,x2)[0] - ret; //licit because WS 
                                                    //is up to date
}

  bool GaussianProcessInterpolation::update_flags_mean()
{

    bool ret = mean_function_->has_changed();
    if (ret) mean_function_->update();
    if (flag_m_) flag_m_ = !ret; 
    if (flag_WSIm_) flag_WSIm_ = !ret; 
    IMP_LOG(TERSE, "update_flags_mean: ret " << ret 
            << " flag_m_ " << flag_m_
            << " flag_WSIm_ " << flag_WSIm_ << std::endl );
    return ret;
}

  bool GaussianProcessInterpolation::update_flags_covariance()
{
    bool ret = covariance_function_->has_changed();
    if (ret) covariance_function_->update();
    if (flag_WS_) flag_WS_ = !ret; 
    if (flag_WSIm_) flag_WSIm_ = !ret; 
    if (flag_W_) flag_W_ = !ret; 
    IMP_LOG(TERSE, "update_flags_covariance: ret " << ret 
            << " flag_WS_ " << flag_WS_ 
            << " flag_WSIm_ " << flag_WSIm_ 
            << " flag_W_ " << flag_W_ 
            << std::endl );
    return ret;
}

  Array1D<double> GaussianProcessInterpolation::get_wx_vector(
                                    std::vector<double> xval)
{
    update_flags_covariance();
    IMP_LOG(TERSE,"  get_wx_vector at q= " << xval[0] << " ");
    wx_ = Array1D<double> (M_);
    for (unsigned i=0; i<M_; i++)
    {
        wx_[i] = (*covariance_function_)(x_[i],xval)[0];
        IMP_LOG(TERSE, wx_[i] << " ");
    }
    IMP_LOG(TERSE, std::endl);
    return wx_;
}

 Array1D<double> GaussianProcessInterpolation::get_WSIm()
{
    IMP_LOG(TERSE, "get_WSIm()" << std::endl);
    update_flags_mean();
    update_flags_covariance();
    if (!flag_WSIm_) 
    {
        IMP_LOG(TERSE, "need to update WSIm_" << std::endl);
        compute_WSIm();
        flag_WSIm_ = true;
    }
    return WSIm_;
}

 void GaussianProcessInterpolation::compute_WSIm()
{
        Array1D<double> I(get_I());
        Array1D<double> m(get_m());
        Array2D<double> WS(get_WS());

        WSIm_ = Array1D<double> (M_);
        IMP_LOG(TERSE, "WSIm ");
        for (unsigned i=0; i<M_; i++)
        {
            WSIm_[i] = 0.0;
            for (unsigned j=0; j<M_; j++)
            {
                WSIm_[i] += WS[i][j] * (I[j] - m[j]);
            }
            IMP_LOG(TERSE, WSIm_[i] << " ");
        }
        IMP_LOG(TERSE, std::endl);
}

  Array1D<double> GaussianProcessInterpolation::get_m()
{
    IMP_LOG(TERSE, "get_m()" << std::endl);
    update_flags_mean();
    if (!flag_m_)
    { 
        IMP_LOG(TERSE, "need to update m" << std::endl);
        compute_m();
        flag_m_ = true;
    }
    return m_;
}

  void GaussianProcessInterpolation::compute_m()
{
    m_ = Array1D<double> (M_);
    for (unsigned i=0; i<M_; i++)
    {
        m_[i] = (*mean_function_)(x_[i])[0];
    }
    IMP_LOG(TERSE, std::endl);
}

  Array2D<double> GaussianProcessInterpolation::get_WS()
{
    IMP_LOG(TERSE, "get_WS()" << std::endl);
    update_flags_covariance();
    if (!flag_WS_)
    { 
        IMP_LOG(TERSE, "need to update (W+S)^{-1}" << std::endl);
        compute_WS();
        flag_WS_ = true;
    }
    return WS_;
}

  void GaussianProcessInterpolation::compute_WS()
{
    //compute W+S
    Array2D<double> WpS = get_W().copy();
    Array2D<double> S(get_S());

    for (unsigned i =0; i<M_; i++) WpS[i][i] += S[i][i];
    IMP_IF_LOG(TERSE) { 
            for (unsigned i=0; i<M_; i++) {
                for (unsigned j=0; j<M_; j++) {
                    IMP_LOG(TERSE, WpS[i][j] << " ");
                }
                IMP_LOG(TERSE, std::endl);
            }
            };
    IMP_LOG(TERSE,"  compute_inverse: Cholesky" << std::endl);
    //compute Cholesky decomp
    Cholesky_.reset(new algebra::internal::JAMA::Cholesky<double> (WpS));
    IMP_USAGE_CHECK(Cholesky_->is_spd(), 
            "Matrix is not symmetric positive definite!");
    //get inverse
    IMP_LOG(TERSE,"  compute_inverse: inverse" << std::endl);
    Array2D<double> id(M_,M_,0.0);
    for (unsigned i=0; i<M_; i++) id[i][i] = 1.0;
    WS_= Cholesky_->solve(id);
    for (unsigned i=0; i<M_; i++) {
        for (unsigned j=0; j<M_; j++) {
            IMP_LOG(TERSE, "WS[" << i << "][" << j << "]: " 
                                << WS_[i][j] << std::endl);
        }
    }
}

  Array2D<double> GaussianProcessInterpolation::get_W()
{
    IMP_LOG(TERSE, "get_W()" << std::endl);
    update_flags_covariance();
    if (!flag_W_)
    { 
        IMP_LOG(TERSE, "need to update W" << std::endl);
        compute_W();
        flag_W_ = true;
    }
    return W_;
}

  void GaussianProcessInterpolation::compute_W()
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

  void GaussianProcessInterpolation::do_show(std::ostream &out) const
{
    out << "Interpolation via gaussian process" << std::endl;
}

IMPISD_END_NAMESPACE

