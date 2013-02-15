/**
 *  \file GaussianProcessInterpolation.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
                               Floats sample_mean,
                               Floats sample_std,
                               unsigned n_obs,
                               UnivariateFunction *mean_function,
                               BivariateFunction *covariance_function,
                               Particle *sigma,
                               double sparse_cutoff) :
        Object("GaussianProcessInterpolation%1%"), x_(x), n_obs_(n_obs),
        mean_function_(mean_function),
        covariance_function_(covariance_function),
        sigma_(sigma),
        cutoff_(sparse_cutoff)
{
    //O(M)
    //store dimensions
    M_ = x.size();
    N_ = x[0].size();
    sigma_val_ = Scale(sigma_).get_nuisance();
    //basic checks
    IMP_USAGE_CHECK(sample_mean.size() == M_,
            "sample_mean should have the same size as x");
    IMP_USAGE_CHECK(sample_std.size() == M_,
            "sample_std should have the same size as x");
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
    IMP_IF_CHECK(USAGE_AND_INTERNAL) { Scale::decorate_particle(sigma); }
    // set all flags to false = need update.
    force_mean_update();
    force_covariance_update();
    //compute needed matrices
    compute_I(sample_mean);
    compute_S(sample_std);
}

  void GaussianProcessInterpolation::force_mean_update()
{
    flag_m_ = false;
    flag_m_gpir_ = false;
    flag_OmiIm_ = false;
}

  void GaussianProcessInterpolation::force_covariance_update()
{
    flag_Omi_ = false;
    flag_OmiIm_ = false;
    flag_W_ = false;
    flag_Omega_ = false;
    flag_Omega_gpir_ = false; // the gpi restraint needs to know when
                          // to update the mvn's Sigma:=Omega matrix.
}

  void GaussianProcessInterpolation::compute_I(Floats mean)
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

  void GaussianProcessInterpolation::compute_S(Floats std)
    {
        //if you modify this routine so that
        //S is not diagonal check the GPIR to make sure it still needs
        //to call set_W_nonzero of MVN.
        VectorXd v(M_);
        IMP_LOG_TERSE( "S: ");
        for (unsigned i=0; i<M_; i++)
        {
            v(i) = IMP::square(std[i]);
            IMP_LOG_TERSE( v(i) << " ");
        }
        S_ = v.asDiagonal();
    IMP_LOG_TERSE( std::endl);
    }

  FloatsList GaussianProcessInterpolation::get_data_abscissa() const
{
    return x_;
}

  Floats GaussianProcessInterpolation::get_data_mean() const
{
    Floats ret;
    VectorXd I(get_I());
    for (unsigned i=0; i<M_; i++) ret.push_back(I(i));
    return ret;
}

  FloatsList GaussianProcessInterpolation::get_data_variance() const
{
    FloatsList ret;
    MatrixXd S(get_S());
    for (unsigned i=0; i<M_; i++)
    {
        Floats val;
        for (unsigned j=0; j<M_; j++) val.push_back(S(i,j));
        ret.push_back(val);
    }
    return ret;
}

  double GaussianProcessInterpolation::get_posterior_mean(Floats x) const
{
   // std::cerr << "posterior mean at q=" << x(0) << std::endl;
    VectorXd wx(get_wx_vector(x));
    VectorXd OmiIm(get_OmiIm());
    double ret = wx.transpose()*OmiIm;
    /*std::cerr << "wx : ";
    for (unsigned i=0; i<M_; i++) std::cerr << wx(i) << " ";
    std::cerr << std::endl << "OmiIm : ";
    for (unsigned i=0; i<M_; i++) std::cerr << OmiIm(i) << " ";
    std::cerr << std::endl << "mean func: " << (*mean_function_)(x)[0]
        << std::endl;
    std::cerr << "product " << ret << std::endl;*/
    return ret + (*mean_function_)(x)[0]; //licit because OmiIm is up to date
}

  double GaussianProcessInterpolation::get_posterior_covariance(
          Floats x1, Floats x2) const
{
    //std::cerr << "posterior covariance at q=" << x1(0) << std::endl;
    VectorXd wx2(get_wx_vector(x2));
    MatrixXd Omi(get_Omi());
    VectorXd wx1;
    if (x1 != x2)
    {
         wx1=get_wx_vector(x1);
    } else {
         wx1=wx2;
    }
    double ret = wx1.transpose()*Omi*wx2;
    return (*covariance_function_)(x1,x2)[0] - ret; //licit because Omi
                                                    //is up to date
}

  MatrixXd GaussianProcessInterpolation::get_posterior_covariance_matrix(
          FloatsList x) const
{
    unsigned N(x.size());
    MatrixXd Wpri(M_,N);
    for (unsigned i=0; i<N; i++) Wpri.col(i) = get_wx_vector(x[i]);
    MatrixXd Omi(get_Omi()); // we can now use covariance_function_
    MatrixXd Wpost((*covariance_function_)(x));
    return Wpost - Wpri.transpose()*Omi*Wpri;
}

  FloatsList GaussianProcessInterpolation::get_posterior_covariance_matrix(
          FloatsList x, bool) const
{
      FloatsList ret;
      MatrixXd mat(get_posterior_covariance_matrix(x));
      unsigned N = mat.rows();
      for (unsigned i=0; i < N; i++)
      {
          Floats tmp;
          for (unsigned j=0; j < N; j++) tmp.push_back(mat(i,j));
          ret.push_back(tmp);
      }
      return ret;
}

VectorXd GaussianProcessInterpolation::get_posterior_covariance_derivative(
        Floats x) const
{
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    //get how many and which particles are optimized
    unsigned mnum = get_number_of_m_particles();
    std::vector<bool> mopt;
    unsigned mnum_opt = 0;
    for (unsigned i=0; i<mnum; i++)
    {
        mopt.push_back(get_m_particle_is_optimized(i));
        if (mopt.back()) mnum_opt++;
    }
    unsigned Onum = get_number_of_Omega_particles();
    std::vector<bool> Oopt;
    unsigned Onum_opt = 0;
    for (unsigned i=0; i<Onum; i++)
    {
        Oopt.push_back(get_Omega_particle_is_optimized(i));
        if (Oopt.back()) Onum_opt++;
    }
    unsigned num_opt = mnum_opt + Onum_opt;
    unsigned sigma_opt = Oopt[0] ? 1 : 0;

    VectorXd ret(num_opt);
    ret.head(mnum_opt+sigma_opt).setZero();
    // build vector of dcov(q,q)/dparticles
    FloatsList xv;
    xv.push_back(x);
    for (unsigned i=0, j=0; i<Onum-1; i++) // skip sigma
        if (Oopt[i+1])
            ret(mnum_opt + sigma_opt + j++) =
                covariance_function_->get_derivative_matrix(i, xv)(0,0);

    //add dcov/dw(q) * dw(q)/dparticles
    MatrixXd dwqdp(M_,num_opt);
    for (unsigned i=0, j=0; i<mnum+Onum; i++)
        if ( ((i < mnum) && mopt[i]) || (i >= mnum && Oopt[i-mnum]) )
            dwqdp.col(j++) = get_wx_vector_derivative(x, i);
    ret += dwqdp.transpose()*get_dcov_dwq(x);

    // add dcov/dOm * dOm/dparticles
    MatrixXd dcovdOm(get_dcov_dOm(x));
    for (unsigned i=0, j=0; i<Onum; i++)
        if (Oopt[i])
            ret.tail(Onum_opt)(j++) +=
                (dcovdOm.transpose()*get_Omega_derivative(i)).trace();

    return ret;
}

  Floats GaussianProcessInterpolation::get_posterior_covariance_derivative(
          Floats x, bool) const
{
      VectorXd mat(get_posterior_covariance_derivative(x));
      Floats tmp;
      for (unsigned j=0; j < mat.rows(); j++) tmp.push_back(mat(j));
      return tmp;
}

MatrixXd GaussianProcessInterpolation::get_posterior_covariance_hessian(
        Floats x) const
{
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    //get how many and which particles are optimized
    unsigned mnum = get_number_of_m_particles();
    std::vector<bool> mopt;
    unsigned mnum_opt = 0;
    for (unsigned i=0; i<mnum; i++)
    {
        mopt.push_back(get_m_particle_is_optimized(i));
        if (mopt.back()) mnum_opt++;
    }
    unsigned Onum = get_number_of_Omega_particles();
    std::vector<bool> Oopt;
    unsigned Onum_opt = 0;
    for (unsigned i=0; i<Onum; i++)
    {
        Oopt.push_back(get_Omega_particle_is_optimized(i));
        if (Oopt.back()) Onum_opt++;
    }
    //total number of optimized particles
    unsigned num_opt = mnum_opt + Onum_opt;
    //whether sigma is optimized
    unsigned sigma_opt = Oopt[0] ? 1 : 0;
    //cov_opt: number of optimized covariance particles without counting sigma
    unsigned cov_opt = Onum_opt - sigma_opt;

    //init matrix and fill with zeros at mean particle's indices
    //dprior_cov(q,q)/(dsigma d.) is also zero
    MatrixXd ret(MatrixXd::Zero(num_opt,num_opt));
    // build vector of dcov(q,q)/dp1dp2 with p1 and p2 covariance particles
    FloatsList xv;
    xv.push_back(x);
    FloatsList tmplist;
    for (unsigned pa=0; pa<Onum; ++pa){
        if (!Oopt[pa]) continue; //skip not optimized particles
        if (pa==0) continue; // skip sigma even when it is optimized
        Floats tmp;
        for (unsigned pb=pa; pb<Onum; ++pb){
            if (!Oopt[pb]) continue; //skip not optimized particles
            //sigma has already been skipped
            tmp.push_back(
                covariance_function_->get_second_derivative_matrix(
                        pa-1, pb-1, xv)(0,0) );
        }
        tmplist.push_back(tmp);
    }
    for (unsigned pa_opt=0; pa_opt<cov_opt; pa_opt++)
        for (unsigned pb_opt=pa_opt; pb_opt<cov_opt; pb_opt++)
            ret.bottomRightCorner(cov_opt, cov_opt)(pa_opt,pb_opt)
                = tmplist[pa_opt][pb_opt-pa_opt];

    //compute and store w(q) derivatives (skip mean particles)
    MatrixXd dwqdp(M_,Onum_opt);
    for (unsigned i=0, j=0; i<Onum; i++)
        if (Oopt[i]) dwqdp.col(j++) = get_wx_vector_derivative(x, i+mnum);
    //add d2cov/(dw(q)_k * dw(q)_l) * dw(q)^k/dp_i * dw(q)^l/dp_j
    ret.bottomRightCorner(cov_opt, cov_opt).noalias()
        += dwqdp.rightCols(cov_opt).transpose()
            *get_d2cov_dwq_dwq()
            *dwqdp.rightCols(cov_opt);

    //compute and store Omega derivatives (skip mean particles)
    std::vector<MatrixXd> dOmdp;
    for (unsigned i=0; i<Onum; i++){
        if (Oopt[i]) dOmdp.push_back(get_Omega_derivative(i));
    }
    //add d2cov/(dOm_kl * dOm_mn) * dOm^kl/dp_i * dOm^mn/dp_j
    std::vector< std::vector<MatrixXd> > d2covdo;
    for (unsigned m=0; m<M_; m++){
        std::vector<MatrixXd> tmp;
        for (unsigned n=0; n<M_; n++)
            tmp.push_back(get_d2cov_dOm_dOm(x, m, n));
        d2covdo.push_back(tmp);
    }

    for (unsigned i=0; i<Onum_opt; i++){
        MatrixXd tmp(M_,M_);
        for (unsigned m=0; m<M_; ++m)
            for (unsigned n=0; n<M_; ++n)
                tmp(m,n) = (d2covdo[m][n].transpose()*dOmdp[i]).trace();
        for (unsigned j=i; j<Onum_opt; j++)
            ret.bottomRightCorner(Onum_opt, Onum_opt)(i,j)
                += (dOmdp[j].transpose()*tmp).trace();
    }
    for (unsigned i=0; i<d2covdo.size(); i++)
        for (unsigned j=0; j<d2covdo[i].size(); j++)
            d2covdo[i][j].resize(0,0);

    //compute cross-term
    std::vector<MatrixXd> d2covdwdo;
    for (unsigned k=0; k<M_; k++)
        d2covdwdo.push_back(get_d2cov_dwq_dOm(x, k));
    //compute derivative contribution into temporary
    MatrixXd tmpH(Onum_opt, Onum_opt);

    for (unsigned i=0; i<Onum_opt; i++){
        VectorXd tmp(M_);
        for (unsigned k=0; k<M_; k++)
            tmp(k) = (d2covdwdo[k].transpose()*dOmdp[i]).trace();
        for (unsigned j=0; j<Onum_opt; j++)
                tmpH(i,j) = dwqdp.col(j).transpose()*tmp;
    }
    ret.bottomRightCorner(Onum_opt,Onum_opt) += tmpH+tmpH.transpose();

    //deallocate unused stuff
    tmpH.resize(0,0);
    for (unsigned i=0; i<d2covdwdo.size(); i++)
        d2covdwdo[i].resize(0,0);
    for (unsigned i=0; i<dOmdp.size(); i++)
        dOmdp[i].resize(0,0);
    dwqdp.resize(0,0);

    //dcov/dw_k * d2w^k/(dp_i dp_j)
    VectorXd dcwq(get_dcov_dwq(x));
    for (unsigned i=0; i<cov_opt; i++)
        for (unsigned j=i; j<cov_opt; j++)
            ret.bottomRightCorner(cov_opt,cov_opt)(i,j)
                += dcwq.transpose()*get_wx_vector_second_derivative(
                        x, mnum+1+i, mnum+1+j);
    dcwq.resize(0,0);

    //dcov/dOm_kl * d2Om^kl/(dp_i dp_j), zero when p_i or p_j is sigma or mean
    MatrixXd dcOm(get_dcov_dOm(x));
    for (unsigned i=0; i<cov_opt; i++)
        for (unsigned j=i; j<cov_opt; j++)
            ret.bottomRightCorner(cov_opt,cov_opt)(i,j)
                += (dcOm.transpose()*get_Omega_second_derivative(
                            i+1,j+1)).trace();
    dcOm.resize(0,0);

    //return as symmetric matrix
    for (unsigned i=mnum_opt; i<num_opt; ++i)
        for (unsigned j=mnum_opt+1; j<num_opt; ++j)
            ret(j,i) = ret(i,j);

    return ret;
}

  FloatsList GaussianProcessInterpolation::get_posterior_covariance_hessian(
          Floats x, bool) const
{
      MatrixXd mat(get_posterior_covariance_hessian(x));
      FloatsList ret;
      for (unsigned j=0; j < mat.rows(); j++){
          Floats tmp;
          for (unsigned i=0; i < mat.cols(); i++) tmp.push_back(mat(i,j));
          ret.push_back(tmp);
      }
      return ret;
}


  void GaussianProcessInterpolation::update_flags_mean()
{

    bool ret = mean_function_->has_changed();
    if (ret) mean_function_->update();
    if (flag_m_) flag_m_ = !ret;
    if (flag_m_gpir_) flag_m_gpir_ = !ret;
    if (flag_OmiIm_) flag_OmiIm_ = !ret;
    IMP_LOG_TERSE( "update_flags_mean: ret " << ret
            << " flag_m_ " << flag_m_
            << " flag_m_gpir_ " << flag_m_gpir_
            << " flag_OmiIm_ " << flag_OmiIm_ << std::endl );
}

  void GaussianProcessInterpolation::update_flags_covariance()
{
    bool ret = covariance_function_->has_changed();
    if (ret) covariance_function_->update();
    if (flag_Omi_) flag_Omi_ = !ret;
    if (flag_OmiIm_) flag_OmiIm_ = !ret;
    if (flag_W_) flag_W_ = !ret;
    if (flag_Omega_) flag_Omega_ = !ret;
    if (flag_Omega_gpir_) flag_Omega_gpir_ = !ret;
    //the case of Omega is a bit peculiar
    //Omega = W + sigma*S/N so it depends on W and sigma.
    //W is treated by flag_W_ and sigma is updated here
    //since it's just a double
    double si = Scale(sigma_).get_nuisance();
    if (std::abs(sigma_val_ - si) > 1e-7)
    {
        //sigma has changed, update its value
        sigma_val_ = si;
        //and mark Omega as outdated
        flag_Omega_ = false;
        flag_Omega_gpir_ = false;
        flag_Omi_ = false;
        flag_OmiIm_ = false;
    }
    IMP_LOG_TERSE( "update_flags_covariance: ret " << ret
            << " flag_Omi_ " << flag_Omi_
            << " flag_OmiIm_ " << flag_OmiIm_
            << " flag_W_ " << flag_W_
            << " flag_Omega_ " << flag_Omega_
            << " flag_Omega_gpir_ " << flag_Omega_gpir_
            << std::endl );
}

  VectorXd GaussianProcessInterpolation::get_wx_vector(Floats xval) const
{
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    IMP_LOG_TERSE("  get_wx_vector at q= " << xval[0] << " ");
    VectorXd wx(M_);
    for (unsigned i=0; i<M_; i++)
    {
        wx(i) = (*covariance_function_)(x_[i],xval)[0];
        IMP_LOG_TERSE( wx(i) << " ");
    }
    IMP_LOG_TERSE( std::endl);
    return wx;
}

  VectorXd GaussianProcessInterpolation::get_wx_vector_derivative(
                                    Floats xval, unsigned p) const
{
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    unsigned nm = get_number_of_m_particles();
    //derivative wrt mean particles and/or sigma is zero
    if (p <= nm) return VectorXd::Zero(M_);
    VectorXd ret(M_);
    for (unsigned i=0; i<M_; i++){
        FloatsList xv;
        xv.push_back(xval);
        xv.push_back(x_[i]);
        ret(i) = covariance_function_->get_derivative_matrix(p-(nm+1),xv)(0,1);
    }
    return ret;
}

  VectorXd GaussianProcessInterpolation::get_wx_vector_second_derivative(
                                    Floats xval, unsigned i, unsigned j) const
{
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    unsigned nm = get_number_of_m_particles();
    //derivative wrt mean particles and/or sigma is zero
    if (i <= nm || j <= nm) return VectorXd::Zero(M_);
    VectorXd ret(M_);
    for (unsigned q=0; q<M_; q++){
        FloatsList xv;
        xv.push_back(xval);
        xv.push_back(x_[q]);
        ret(q) = covariance_function_->get_second_derivative_matrix(
                    i-(nm+1), j-(nm+1), xv)(0,1);
    }
    return ret;
}


 VectorXd GaussianProcessInterpolation::get_OmiIm() const
{
    IMP_LOG_TERSE( "get_OmiIm()" << std::endl);
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_mean();
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    if (!flag_OmiIm_)
    {
        IMP_LOG_TERSE( "need to update OmiIm_" << std::endl);
        const_cast<GaussianProcessInterpolation *>(this)->compute_OmiIm();
        const_cast<GaussianProcessInterpolation *>(this)->flag_OmiIm_ = true;
        IMP_LOG_TERSE( "done updating OmiIm_" << std::endl);
    }
    return OmiIm_;
}

 void GaussianProcessInterpolation::compute_OmiIm()
{
        VectorXd I(get_I());
        VectorXd m(get_m());
        MatrixXd Omi(get_Omi());
        IMP_LOG_TERSE( "OmiIm ");
        OmiIm_ = Omi*(I-m);
        IMP_LOG_TERSE( std::endl);
}

  VectorXd GaussianProcessInterpolation::get_m() const
{
    IMP_LOG_TERSE( "get_m()" << std::endl);
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_mean();
    if (!flag_m_)
    {
        IMP_LOG_TERSE( "need to update m" << std::endl);
        const_cast<GaussianProcessInterpolation *>(this)->compute_m();
        const_cast<GaussianProcessInterpolation *>(this)->flag_m_ = true;
        IMP_LOG_TERSE( "done updating m" << std::endl);
    }
    return m_;
}

  void GaussianProcessInterpolation::compute_m()
{
    m_ = (*mean_function_)(x_);
}

unsigned
GaussianProcessInterpolation::get_number_of_m_particles() const
{
    return mean_function_->get_number_of_particles();
}

bool
GaussianProcessInterpolation::get_m_particle_is_optimized(unsigned i) const
{
    return mean_function_->get_particle_is_optimized(i);
}

VectorXd GaussianProcessInterpolation::get_m_derivative(unsigned particle) const
{
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_mean();
    return mean_function_->get_derivative_vector(particle, x_);
}

VectorXd GaussianProcessInterpolation::get_m_second_derivative(
        unsigned p1, unsigned p2) const
{
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_mean();
    return mean_function_->get_second_derivative_vector(p1, p2, x_);
}

void GaussianProcessInterpolation::add_to_m_particle_derivative(
        unsigned particle, double value, DerivativeAccumulator &accum)
{
    mean_function_->add_to_particle_derivative(particle, value, accum);
}

MatrixXd GaussianProcessInterpolation::get_Omega() const
{
    IMP_LOG_TERSE( "get_Omega()" << std::endl);
    //updates sigma as well
    const_cast<GaussianProcessInterpolation*>(this)->update_flags_covariance();
    if (!flag_Omega_)
    {
        IMP_LOG_TERSE( "need to update Omega" << std::endl);
        const_cast<GaussianProcessInterpolation *>(this)->compute_Omega();
        const_cast<GaussianProcessInterpolation *>(this)->flag_Omega_ = true;
        //leave flag_Omega_gpir_ to false so that the gpir is notified
        //if it wants to update some stuff on its own.
        IMP_LOG_TERSE( "done updating Omega" << std::endl);
    }
    return Omega_;
}

void GaussianProcessInterpolation::compute_Omega()
{
    //sigma_val_ is up-to-date because update_flags_covariance was just called
    Omega_ = get_W() + sigma_val_ * MatrixXd(get_S())/n_obs_;
}

unsigned
GaussianProcessInterpolation::get_number_of_Omega_particles() const
{
    //sigma is optimized locally
    return covariance_function_->get_number_of_particles() + 1;
}

bool
GaussianProcessInterpolation::get_Omega_particle_is_optimized(unsigned i) const
{
    //sigma is optimized locally
    if (i==0) {
        return Scale(sigma_).get_nuisance_is_optimized();
    } else {
        return covariance_function_->get_particle_is_optimized(i-1);
    }
}

MatrixXd
GaussianProcessInterpolation::get_Omega_derivative(unsigned particle) const
{
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    //Omega = W + sigma*S/n_obs
    if (particle == 0)
    {
        //sigma
        return MatrixXd(get_S())/n_obs_;
    } else {
        return covariance_function_->get_derivative_matrix(particle-1, x_);
    }
}

MatrixXd GaussianProcessInterpolation::get_Omega_second_derivative(
        unsigned p1, unsigned p2) const
{
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    //Omega = W + sigma*S/n_obs
    if (p1 == 0 || p2 == 0)
    {
        //sigma
        return MatrixXd::Zero(M_,M_);
    } else {
        return covariance_function_->get_second_derivative_matrix(
                p1-1, p2-1, x_);
    }
}

void GaussianProcessInterpolation::add_to_Omega_particle_derivative(
        unsigned particle, double value, DerivativeAccumulator &accum)
{
    if (particle == 0)
    {
        Scale(sigma_).add_to_nuisance_derivative(value, accum);
    } else {
        covariance_function_->add_to_particle_derivative(particle-1, value,
                accum);
    }
}

  MatrixXd GaussianProcessInterpolation::get_Omi() const
{
    IMP_LOG_TERSE( "get_Omi()" << std::endl);
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    if (!flag_Omi_)
    {
        IMP_LOG_TERSE( "need to update Omi" << std::endl);
        const_cast<GaussianProcessInterpolation *>(this)->compute_Omi();
        const_cast<GaussianProcessInterpolation *>(this)->flag_Omi_ = true;
        IMP_LOG_TERSE( "done updating Omi" << std::endl);
    }
    return Omi_;
}

  void GaussianProcessInterpolation::compute_Omi()
{
    //get Omega=W+S/N
    MatrixXd WpS = get_Omega();
    IMP_LOG_TERSE("  compute_inverse: Cholesky" << std::endl);
    //compute Cholesky decomp
    Eigen::LDLT<MatrixXd> ldlt;
    ldlt.compute(WpS);
    if (!ldlt.isPositive())
            IMP_THROW("Matrix is not positive semidefinite!",
                    ModelException);
    //get inverse
    IMP_LOG_TERSE("  compute_inverse: inverse" << std::endl);
    Omi_= ldlt.solve(MatrixXd::Identity(M_,M_));
}

  MatrixXd GaussianProcessInterpolation::get_W() const
{
    IMP_LOG_TERSE( "get_W()" << std::endl);
    const_cast<GaussianProcessInterpolation *>(this)->update_flags_covariance();
    if (!flag_W_)
    {
        IMP_LOG_TERSE( "need to update W" << std::endl);
        const_cast<GaussianProcessInterpolation *>(this)->compute_W();
        const_cast<GaussianProcessInterpolation *>(this)->flag_W_ = true;
        IMP_LOG_TERSE( "done updating W" << std::endl);
    }
    return W_;
}

  void GaussianProcessInterpolation::compute_W()
{
    W_ = (*covariance_function_)(x_);
}

VectorXd GaussianProcessInterpolation::get_dcov_dwq(Floats q) const
{
    VectorXd wq(get_wx_vector(q));
    MatrixXd Omi(get_Omi());
    return -2*Omi*wq;
}

MatrixXd GaussianProcessInterpolation::get_dcov_dOm(Floats q) const
{
    VectorXd wq(get_wx_vector(q));
    MatrixXd Omi(get_Omi());
    VectorXd ret(Omi*wq);
    return ret*ret.transpose();
}

MatrixXd GaussianProcessInterpolation::get_d2cov_dwq_dwq() const
{
    return -2*get_Omi();
}

MatrixXd GaussianProcessInterpolation::get_d2cov_dwq_dOm(Floats q, unsigned m)
    const
{
    MatrixXd Omi(get_Omi());
    VectorXd wq(get_wx_vector(q));
    VectorXd L(Omi*wq);
    MatrixXd ret(L*Omi.col(m).transpose());
    return ret + ret.transpose();
}

MatrixXd GaussianProcessInterpolation::get_d2cov_dOm_dOm(Floats q,
        unsigned m, unsigned n) const
{
    MatrixXd Omi(get_Omi());
    VectorXd wq(get_wx_vector(q));
    VectorXd L(Omi*wq);
    MatrixXd tmp(Omi.col(m)*L.transpose());
    return -L(n)*(tmp + tmp.transpose());
}

/*
  MatrixXd GaussianProcessInterpolation::get_posterior_mean_hessian(Floats x,
          unsigned p1, unsigned p2) const
{
    return MatrixXd::Zero(1,1);
}

  MatrixXd GaussianProcessInterpolation::get_posterior_covariance_hessian(
          Floats x, unsigned p1, unsigned p2) const
{
    return MatrixXd::Zero(1,1);
}
*/
  void GaussianProcessInterpolation::do_show(std::ostream &out) const
{
    out << "Interpolation via gaussian process" << std::endl;
}

  ParticlesTemp GaussianProcessInterpolation::get_input_particles() const
{
    ParticlesTemp ret;
    ParticlesTemp ret1 = mean_function_->get_input_particles();
    ret.insert(ret.end(),ret1.begin(),ret1.end());
    ret.push_back(sigma_);
    ParticlesTemp ret2 = covariance_function_->get_input_particles();
    ret.insert(ret.end(),ret2.begin(),ret2.end());
    return ret;
}

  ContainersTemp GaussianProcessInterpolation::get_input_containers() const
{
    ContainersTemp ret;
    ContainersTemp ret1 = mean_function_->get_input_containers();
    ret.insert(ret.end(),ret1.begin(),ret1.end());
    ContainersTemp ret2 = covariance_function_->get_input_containers();
    ret.insert(ret.end(),ret2.begin(),ret2.end());
    return ret;
}
IMPISD_END_NAMESPACE
