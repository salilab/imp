/**
 *  \file GaussianProcessInterpolationSparse.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/GaussianProcessInterpolationSparse.h>

#ifdef IMP_ISD_USE_CHOLMOD

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

    GaussianProcessInterpolationSparse::GaussianProcessInterpolationSparse(
                               FloatsList x,
                               Floats sample_mean,
                               Floats sample_std,
                               Ints n_obs,
                               UnivariateFunction *mean_function,
                               BivariateFunction *covariance_function,
                               double cutoff) :
        Object("GaussianProcessInterpolationSparse%1%"), x_(x), n_obs_(n_obs),
        mean_function_(mean_function),
        covariance_function_(covariance_function),
        cutoff_(cutoff)
{
    c_=&Common_;
    WS_=nullptr;
    WSIm_=nullptr;
    L_=nullptr;
    cholmod_start(c_);
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

  void GaussianProcessInterpolationSparse::force_mean_update()
{
    flag_m_ = false;
    flag_m_gpir_ = false;
    flag_WSIm_ = false;
}

  void GaussianProcessInterpolationSparse::force_covariance_update()
{
    flag_WS_ = false;
    flag_WSIm_ = false;
    flag_W_ = false;
    flag_W_gpir_ = false; // the gpi restraint needs to know when
                          // to update the mvn's Sigma:=W matrix.
}

  void GaussianProcessInterpolationSparse::compute_I(Floats mean)
{
    I_ = VectorXd (M_);
    IMP_LOG_TERSE( "I: ");
    for (unsigned i=0; i<M_; i++)
    {
        I_(i) = mean[i];
        IMP_LOG_TERSE( I_(i) << " ");
    }
    IMP_LOG_TERSE( std::endl);
}

  void GaussianProcessInterpolationSparse::compute_S(Floats std,
          Ints n)
    {
        //if you modify this routine so that
        //S is not diagonal check the GPIR to make sure it still needs
        //to call set_W_nonzero of MVN.
        VectorXd v(M_);
        IMP_LOG_TERSE( "S: ");
        S_ = SparseMatrix<double>(M_,M_);
        S_.reserve(M_);
        for (unsigned i=0; i<M_; i++)
        {
            double v = IMP::square(std[i])/double(n[i]);
            IMP_LOG_TERSE( v << " ");
            S_.startVec(i);
            S_.insertBack(i,i) = v;
        }
        S_.finalize();
    IMP_LOG_TERSE( std::endl);
    }

  double GaussianProcessInterpolationSparse::get_posterior_mean(Floats x)
{
   // std::cerr << "posterior mean at q=" << x(0) << std::endl;
    cholmod_sparse *wx(get_wx_vector(x));
    cholmod_dense *WSIm(get_WSIm());
    static double one[2]={1,0};
    static double zero[2]={0,0};
    cholmod_dense *retmat = cholmod_zeros(1,1,CHOLMOD_REAL,c_);
    cholmod_sdmult(wx, 1, one, zero, WSIm, retmat, c_);
    //std::cerr << "wx : ";
    //for (unsigned i=0; i<M_; i++) std::cerr << wx(i) << " ";
    //std::cerr << std::endl << "WSIm : ";
    //for (unsigned i=0; i<M_; i++) std::cerr << WSIm(i) << " ";
    //std::cerr << std::endl << "mean func: " << (*mean_function_,x,0)
    //    << std::endl;
    //licit because WSIm is up to date
    double ret = ((double*)retmat->x)[0] + (*mean_function_)(x)[0];
    cholmod_free_dense(&retmat, c_);
    return ret;
}

  double GaussianProcessInterpolationSparse::get_posterior_covariance(
          Floats x1, Floats x2)
{
    //std::cerr << "posterior covariance at q=" << x1(0) << std::endl;
    cholmod_sparse *wx2(get_wx_vector(x2));
    cholmod_sparse *WS(get_WS());
    cholmod_sparse *tmp = cholmod_ssmult(WS,wx2,0,1,0,c_);
    cholmod_sparse *wx1;
    if (x1 != x2) wx2 = get_wx_vector(x1);
     wx1 = cholmod_transpose(wx2,1,c_);
    cholmod_sparse *result = cholmod_ssmult(wx1, tmp, 0, 1, 0, c_);
    double ret=0;
    if (result->x) ret = ((double*)result->x)[0];
    cholmod_free_sparse(&wx1, c_);
    cholmod_free_sparse(&result, c_);
    return (*covariance_function_)(x1,x2)[0] - ret; //licit because WS
                                                    //is up to date
}

  void GaussianProcessInterpolationSparse::update_flags_mean()
{

    bool ret = mean_function_->has_changed();
    if (ret) mean_function_->update();
    if (flag_m_) flag_m_ = !ret;
    if (flag_m_gpir_) flag_m_gpir_ = !ret;
    if (flag_WSIm_) flag_WSIm_ = !ret;
    IMP_LOG_TERSE( "update_flags_mean: ret " << ret
            << " flag_m_ " << flag_m_
            << " flag_m_gpir_ " << flag_m_gpir_
            << " flag_WSIm_ " << flag_WSIm_ << std::endl );
}

  void GaussianProcessInterpolationSparse::update_flags_covariance()
{
    bool ret = covariance_function_->has_changed();
    if (ret) covariance_function_->update();
    if (flag_WS_) flag_WS_ = !ret;
    if (flag_WSIm_) flag_WSIm_ = !ret;
    if (flag_W_) flag_W_ = !ret;
    if (flag_W_gpir_) flag_W_gpir_ = !ret;
    IMP_LOG_TERSE( "update_flags_covariance: ret " << ret
            << " flag_WS_ " << flag_WS_
            << " flag_WSIm_ " << flag_WSIm_
            << " flag_W_ " << flag_W_
            << " flag_W_gpir_ " << flag_W_gpir_
            << std::endl );
}

  cholmod_sparse *GaussianProcessInterpolationSparse::get_wx_vector(
                                    Floats xval)
{
    update_flags_covariance();
    IMP_LOG_TERSE("  get_wx_vector at q= " << xval[0] << " ");
    wx_ = SparseMatrix<double> (M_,1);
    wx_.startVec(0);
    for (unsigned i=0; i<M_; i++)
    {
        double val = (*covariance_function_)(x_[i],xval)[0];
        if (std::abs(val) > cutoff_)
        {
            wx_.insertBack(i,0) = val;
            IMP_LOG_TERSE( val << " ");
        } else {
            IMP_LOG_TERSE( "0 ");
        }
    }
    wx_.finalize();
    cwx_ = Eigen::viewAsCholmod(wx_);
    IMP_LOG_TERSE( std::endl);
    return &cwx_;
}

 cholmod_dense *GaussianProcessInterpolationSparse::get_WSIm()
{
    IMP_LOG_TERSE( "get_WSIm()" << std::endl);
    update_flags_mean();
    update_flags_covariance();
    if (!flag_WSIm_)
    {
        IMP_LOG_TERSE( "need to update WSIm_" << std::endl);
        compute_WSIm();
        flag_WSIm_ = true;
    }
    return WSIm_;
}

 void GaussianProcessInterpolationSparse::compute_WSIm()
{
        VectorXd eIm(get_I()-get_m());
        IMP_LOG_TERSE( "compute_WSIm ");
        cholmod_dense Im = Eigen::viewAsCholmod(eIm);
        cholmod_factor *L = get_L();
        if (WSIm_) cholmod_free_dense(&WSIm_, c_);
        WSIm_ = cholmod_solve(CHOLMOD_A, L, &Im, c_);
}

  VectorXd GaussianProcessInterpolationSparse::get_m()
{
    IMP_LOG_TERSE( "get_m()" << std::endl);
    update_flags_mean();
    if (!flag_m_)
    {
        IMP_LOG_TERSE( "need to update m" << std::endl);
        compute_m();
        flag_m_ = true;
    }
    return m_;
}

  void GaussianProcessInterpolationSparse::compute_m()
{
    m_ = VectorXd (M_);
    for (unsigned i=0; i<M_; i++)
    {
        m_(i) = (*mean_function_)(x_[i])[0];
    }
    IMP_LOG_TERSE( std::endl);
}

  cholmod_sparse *GaussianProcessInterpolationSparse::get_WS()
{
    IMP_LOG_TERSE( "get_WS()" << std::endl);
    update_flags_covariance();
    if (!flag_WS_)
    {
        IMP_LOG_TERSE( "need to update (W+S)^{-1}" << std::endl);
        compute_WS();
        flag_WS_ = true;
    }
    return WS_;
}

  cholmod_factor *GaussianProcessInterpolationSparse::get_L()
{
    IMP_LOG_TERSE( "get_L()" << std::endl);
    update_flags_covariance();
    if (!flag_WS_)
    {
        IMP_LOG_TERSE( "need to update (W+S)^{-1}" << std::endl);
        compute_WS();
        flag_WS_ = true;
    }
    return L_;
}

  void GaussianProcessInterpolationSparse::compute_WS()
{
    //compute W+S
    SparseMatrix<double> WpS(get_W() + get_S());
    cholmod_sparse cWpS = Eigen::viewAsCholmod(
            WpS.selfadjointView<Eigen::Upper>());
    IMP_LOG_TERSE("  compute_inverse: Cholesky" << std::endl);
    //compute Cholesky decomp: LDLT
    c_->final_asis=1;
    c_->supernodal=CHOLMOD_SIMPLICIAL;
    if (L_) cholmod_free_factor(&L_, c_);
    L_ = cholmod_analyze(&cWpS, c_);
    int success = cholmod_factorize(&cWpS, L_, c_);
    if (success == 0 || L_->minor < L_->n)
            IMP_THROW("Matrix is not positive semidefinite!",
                    ModelException);
    //get inverse
    IMP_LOG_TERSE("  compute_inverse: inverse" << std::endl);
    cholmod_sparse *cid = cholmod_speye(M_,M_,CHOLMOD_REAL,c_);
    if (WS_) cholmod_free_sparse(&WS_, c_);
    WS_ = cholmod_spsolve(CHOLMOD_A, L_, cid, c_);
    cholmod_free_sparse(&cid,c_);
}

  SparseMatrix<double> GaussianProcessInterpolationSparse::get_W()
{
    IMP_LOG_TERSE( "get_W()" << std::endl);
    update_flags_covariance();
    if (!flag_W_)
    {
        IMP_LOG_TERSE( "need to update W" << std::endl);
        compute_W();
        flag_W_ = true;
    }
    return W_;
}

  void GaussianProcessInterpolationSparse::compute_W()
{
    W_ = SparseMatrix<double> (M_,M_);
    for (unsigned j=0; j<M_; j++)
    {
        W_.startVec(j);
        for (unsigned i=0; i<=j; i++)
        {
            double val = (*covariance_function_)(x_[i],x_[j])[0];
            if (std::abs(val) > cutoff_)
                W_.insertBack(i,j) = val;
        }
    }
    W_.finalize();
}

IMPISD_END_NAMESPACE

#endif /* IMP_ISD_USE_CHOLMOD */
