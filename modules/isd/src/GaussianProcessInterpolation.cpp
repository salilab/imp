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
    flag_m_gpir_ = false;
    flag_WSIm_ = false;
}

  void GaussianProcessInterpolation::force_covariance_update()
{
    flag_WS_ = false;
    flag_WSIm_ = false;
    flag_W_ = false;
    flag_W_gpir_ = false; // the gpi restraint needs to know when 
                          // to update the mvn's Sigma:=W matrix.
}

  void GaussianProcessInterpolation::compute_I(std::vector<double> mean)
{
    I_ = VectorXd (M_);
    IMP_LOG(TERSE, "I: ");
    for (unsigned i=0; i<M_; i++)
    {
        I_(i) = mean[i];
        IMP_LOG(TERSE, I_(i) << " ");
    }
    IMP_LOG(TERSE, std::endl);
}

  void GaussianProcessInterpolation::compute_S(std::vector<double> std,
          std::vector<int> n) 
    { 
        //if you modify this routine so that
        //S is not diagonal check the GPIR to make sure it still needs
        //to call set_W_nonzero of MVN.
        VectorXd v(M_);
        IMP_LOG(TERSE, "S: ");
        for (unsigned i=0; i<M_; i++) 
        { 
            v(i) = IMP::square(std[i])/double(n[i]); 
            IMP_LOG(TERSE, v(i) << " ");
        } 
        S_ = MatrixXd(v.asDiagonal());
    IMP_LOG(TERSE, std::endl);
    }

  double GaussianProcessInterpolation::get_posterior_mean(std::vector<double> x)
{
   // std::cerr << "posterior mean at q=" << x(0) << std::endl;
    VectorXd wx(get_wx_vector(x));
    VectorXd WSIm(get_WSIm());
    double ret = wx.transpose()*WSIm;
    /*std::cerr << "wx : ";
    for (unsigned i=0; i<M_; i++) std::cerr << wx(i) << " ";
    std::cerr << std::endl << "WSIm : ";
    for (unsigned i=0; i<M_; i++) std::cerr << WSIm(i) << " ";
    std::cerr << std::endl << "mean func: " << (*mean_function_)(x)[0] 
        << std::endl;
    std::cerr << "product " << ret << std::endl;*/
    return ret + (*mean_function_)(x)[0]; //licit because WSIm is up to date
}

  double GaussianProcessInterpolation::get_posterior_covariance(
          std::vector<double> x1, std::vector<double> x2)
{
    //std::cerr << "posterior covariance at q=" << x1(0) << std::endl;
    VectorXd wx2(get_wx_vector(x2));
    MatrixXd WS(get_WS());
    VectorXd wx1;
    if (x1 != x2)
    {
         wx1=get_wx_vector(x1);
    } else {
         wx1=wx2;
    }
    double ret = wx1.transpose()*WS*wx2;
    return (*covariance_function_)(x1,x2)[0] - ret; //licit because WS 
                                                    //is up to date
}

  void GaussianProcessInterpolation::update_flags_mean()
{

    bool ret = mean_function_->has_changed();
    if (ret) mean_function_->update();
    if (flag_m_) flag_m_ = !ret; 
    if (flag_m_gpir_) flag_m_gpir_ = !ret; 
    if (flag_WSIm_) flag_WSIm_ = !ret; 
    IMP_LOG(TERSE, "update_flags_mean: ret " << ret 
            << " flag_m_ " << flag_m_
            << " flag_m_gpir_ " << flag_m_gpir_
            << " flag_WSIm_ " << flag_WSIm_ << std::endl );
}

  void GaussianProcessInterpolation::update_flags_covariance()
{
    bool ret = covariance_function_->has_changed();
    if (ret) covariance_function_->update();
    if (flag_WS_) flag_WS_ = !ret; 
    if (flag_WSIm_) flag_WSIm_ = !ret; 
    if (flag_W_) flag_W_ = !ret; 
    if (flag_W_gpir_) flag_W_gpir_ = !ret; 
    IMP_LOG(TERSE, "update_flags_covariance: ret " << ret 
            << " flag_WS_ " << flag_WS_ 
            << " flag_WSIm_ " << flag_WSIm_ 
            << " flag_W_ " << flag_W_ 
            << " flag_W_gpir_ " << flag_W_gpir_ 
            << std::endl );
}

  VectorXd GaussianProcessInterpolation::get_wx_vector(
                                    std::vector<double> xval)
{
    update_flags_covariance();
    IMP_LOG(TERSE,"  get_wx_vector at q= " << xval[0] << " ");
    wx_ = VectorXd (M_);
    for (unsigned i=0; i<M_; i++)
    {
        wx_(i) = (*covariance_function_)(x_[i],xval)[0];
        IMP_LOG(TERSE, wx_(i) << " ");
    }
    IMP_LOG(TERSE, std::endl);
    return wx_;
}

 VectorXd GaussianProcessInterpolation::get_WSIm()
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
        VectorXd I(get_I());
        VectorXd m(get_m());
        MatrixXd WS(get_WS());
        IMP_LOG(TERSE, "WSIm ");
        WSIm_ = WS*(I-m);
        IMP_LOG(TERSE, std::endl);
}

  VectorXd GaussianProcessInterpolation::get_m()
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
    m_ = VectorXd (M_);
    for (unsigned i=0; i<M_; i++)
    {
        m_(i) = (*mean_function_)(x_[i])[0];
    }
    IMP_LOG(TERSE, std::endl);
}

  MatrixXd GaussianProcessInterpolation::get_WS()
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
    MatrixXd WpS = get_W() + get_S();
    IMP_LOG(TERSE,"  compute_inverse: Cholesky" << std::endl);
    //compute Cholesky decomp
    Eigen::LDLT<MatrixXd> ldlt;
    ldlt.compute(WpS);
    if (!ldlt.isPositive())
            IMP_THROW("Matrix is not positive semidefinite!", 
                    ModelException);
    //get inverse
    IMP_LOG(TERSE,"  compute_inverse: inverse" << std::endl);
    WS_= ldlt.solve(MatrixXd::Identity(M_,M_));
}

  MatrixXd GaussianProcessInterpolation::get_W()
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
    W_ = MatrixXd (M_,M_);
    for (unsigned i=0; i<M_; i++)
    {
        W_(i,i) = (*covariance_function_)(x_[i],x_[i])[0];
        for (unsigned j=i+1; j<M_; j++)
        {
            W_(i,j) = (*covariance_function_)(x_[i],x_[j])[0];
            W_(j,i) = W_(i,j);
        }
    }
}

  void GaussianProcessInterpolation::do_show(std::ostream &out) const
{
    out << "Interpolation via gaussian process" << std::endl;
}

IMPISD_END_NAMESPACE

