/**
 *  \file GaussianProcessInterpolation.cpp
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
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
    IMP_LOG(TERSE, "I: ");
    for (unsigned i=0; i<M_; i++)
    {
        I_(i) = mean[i];
        IMP_LOG(TERSE, I_(i) << " ");
    }
    IMP_LOG(TERSE, std::endl);
}

  void GaussianProcessInterpolation::compute_S(Floats std)
    {
        //if you modify this routine so that
        //S is not diagonal check the GPIR to make sure it still needs
        //to call set_W_nonzero of MVN.
        VectorXd v(M_);
        IMP_LOG(TERSE, "S: ");
        for (unsigned i=0; i<M_; i++)
        {
            v(i) = IMP::square(std[i]);
            IMP_LOG(TERSE, v(i) << " ");
        }
        S_ = v.asDiagonal();
    IMP_LOG(TERSE, std::endl);
    }

  double GaussianProcessInterpolation::get_posterior_mean(Floats x)
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
          Floats x1, Floats x2)
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

  void GaussianProcessInterpolation::update_flags_mean()
{

    bool ret = mean_function_->has_changed();
    if (ret) mean_function_->update();
    if (flag_m_) flag_m_ = !ret;
    if (flag_m_gpir_) flag_m_gpir_ = !ret;
    if (flag_OmiIm_) flag_OmiIm_ = !ret;
    IMP_LOG(TERSE, "update_flags_mean: ret " << ret
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
    }
    IMP_LOG(TERSE, "update_flags_covariance: ret " << ret
            << " flag_Omi_ " << flag_Omi_
            << " flag_OmiIm_ " << flag_OmiIm_
            << " flag_W_ " << flag_W_
            << " flag_Omega_ " << flag_Omega_
            << " flag_Omega_gpir_ " << flag_Omega_gpir_
            << std::endl );
}

  VectorXd GaussianProcessInterpolation::get_wx_vector(
                                    Floats xval)
{
    update_flags_covariance();
    IMP_LOG(TERSE,"  get_wx_vector at q= " << xval[0] << " ");
    wx_.resize(M_);
    for (unsigned i=0; i<M_; i++)
    {
        wx_(i) = (*covariance_function_)(x_[i],xval)[0];
        IMP_LOG(TERSE, wx_(i) << " ");
    }
    IMP_LOG(TERSE, std::endl);
    return wx_;
}

 VectorXd GaussianProcessInterpolation::get_OmiIm()
{
    IMP_LOG(TERSE, "get_OmiIm()" << std::endl);
    update_flags_mean();
    update_flags_covariance();
    if (!flag_OmiIm_)
    {
        IMP_LOG(TERSE, "need to update OmiIm_" << std::endl);
        compute_OmiIm();
        flag_OmiIm_ = true;
        IMP_LOG(TERSE, "done updating OmiIm_" << std::endl);
    }
    return OmiIm_;
}

 void GaussianProcessInterpolation::compute_OmiIm()
{
        VectorXd I(get_I());
        VectorXd m(get_m());
        MatrixXd Omi(get_Omi());
        IMP_LOG(TERSE, "OmiIm ");
        OmiIm_ = Omi*(I-m);
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
        IMP_LOG(TERSE, "done updating m" << std::endl);
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
    return mean_function_->get_derivative_vector(particle, x_);
}

void GaussianProcessInterpolation::add_to_m_particle_derivative(
        unsigned particle, double value, DerivativeAccumulator &accum)
{
    mean_function_->add_to_particle_derivative(particle, value, accum);
}

MatrixXd GaussianProcessInterpolation::get_Omega()
{
    IMP_LOG(TERSE, "get_Omega()" << std::endl);
    update_flags_covariance(); //updates sigma as well
    if (!flag_Omega_)
    {
        IMP_LOG(TERSE, "need to update Omega" << std::endl);
        compute_Omega();
        flag_Omega_ = true;
        //leave flag_Omega_gpir_ to false so that the gpir is notified
        //if it wants to update some stuff on its own.
        IMP_LOG(TERSE, "done updating Omega" << std::endl);
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
    //Omega = W + sigma*S/n_obs
    if (particle == 0)
    {
        //sigma
        return MatrixXd(get_S())/n_obs_;
    } else {
        return covariance_function_->get_derivative_matrix(particle-1, x_);
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

  MatrixXd GaussianProcessInterpolation::get_Omi()
{
    IMP_LOG(TERSE, "get_Omi()" << std::endl);
    update_flags_covariance();
    if (!flag_Omi_)
    {
        IMP_LOG(TERSE, "need to update Omi" << std::endl);
        compute_Omi();
        flag_Omi_ = true;
        IMP_LOG(TERSE, "done updating Omi" << std::endl);
    }
    return Omi_;
}

  void GaussianProcessInterpolation::compute_Omi()
{
    //get Omega=W+S/N
    MatrixXd WpS = get_Omega();
    IMP_LOG(TERSE,"  compute_inverse: Cholesky" << std::endl);
    //compute Cholesky decomp
    Eigen::LDLT<MatrixXd> ldlt;
    ldlt.compute(WpS);
    if (!ldlt.isPositive())
            IMP_THROW("Matrix is not positive semidefinite!",
                    ModelException);
    //get inverse
    IMP_LOG(TERSE,"  compute_inverse: inverse" << std::endl);
    Omi_= ldlt.solve(MatrixXd::Identity(M_,M_));
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
        IMP_LOG(TERSE, "done updating W" << std::endl);
    }
    return W_;
}

  void GaussianProcessInterpolation::compute_W()
{
    W_ = (*covariance_function_)(x_);
}

  FloatsList GaussianProcessInterpolation::get_Hessian()
{
    //collect necessary matrices (except second derivatives)
    MatrixXd Omi(get_Omi());
    VectorXd OmiIm(get_OmiIm());
    //dmean and dcov derivatives of m and Omega wrt particles
    std::vector<VectorXd> dmean; // len(dmean) = number of particles
    std::vector<MatrixXd> dcov;
    std::vector<RowVectorXd> dmom; // trans(dm/dpart) * Omega^{-1}
    for (unsigned i=0;
            i<mean_function_->get_number_of_particles(); i++)
    {
        if (!mean_function_->get_particle_is_optimized(i)) continue;
        dmean.push_back(mean_function_->get_derivative_vector(i, x_));
        dmom.push_back(dmean.back().transpose()*Omi);
        dcov.push_back(MatrixXd::Zero(M_,M_));
    }
    if (Scale(sigma_).get_nuisance_is_optimized())
    {
        dmean.push_back(VectorXd::Zero(M_));
        dcov.push_back(MatrixXd(get_S())/n_obs_); //dOmega/dsigma = S/n
        dmom.push_back(RowVectorXd::Zero(M_));
    }
    for (unsigned i=0;
            i<covariance_function_->get_number_of_particles(); i++)
    {
        if (!covariance_function_->get_particle_is_optimized(i)) continue;
        dmean.push_back(VectorXd::Zero(M_));
        dcov.push_back(covariance_function_->get_derivative_matrix(i, x_));
        dmom.push_back(RowVectorXd::Zero(M_));
    }
    int n_opt = dmean.size(); //total number of optimized particles
    int n_mean = mean_function_->get_number_of_optimized_particles();
    //
    // the Hessian is assumed to be symmetric because all functions are
    // differentiable so d2f/dp1dp2=d2f/dp2dp1
    // further simplifications arise from the fact that particles are assigned
    // to only one function so that second derivatives need to be computed only
    // when p1 and p2 belong to the same function.
    MatrixXd H(n_opt,n_opt);
    H.setZero();
    for (int mu=0; mu<n_opt; mu++)
    {
        for (int nu=mu; nu<n_opt; nu++)
        {
            H(mu,nu) =  dmom[mu] * ( dcov[nu] * OmiIm + dmean[nu] );
            H(mu,nu) += dmom[nu] * dcov[mu] * OmiIm;
            H(mu,nu) += (OmiIm.transpose()*dcov[nu])*Omi*(dcov[mu]*OmiIm);
            H(mu,nu) -= 0.5*(Omi*dcov[nu].transpose()*Omi*dcov[mu]).trace();
            if (nu < n_mean && mu < n_mean) {
                //second derivative for mean particle
                VectorXd secm(
                    mean_function_->get_second_derivative_vector(mu,nu,x_));
                H(mu,nu) += OmiIm.transpose()*secm;
            } else if (mu > n_mean && nu > n_mean) {
                //second derivative for sigma is zero,
                //only covariance particles remain
                MatrixXd secOmega(covariance_function_->
                    get_second_derivative_matrix(mu-(n_mean+1), nu-(n_mean+1),
                        x_));
                H(mu,nu) -= 0.5*
                   ( (OmiIm*OmiIm.transpose() + Omi)*secOmega).trace();
            }
            if (nu != mu) H(nu,mu) = H(mu,nu);
        }
    }
    //convert to FloatsList for SWIG.
    FloatsList Hessian;
    for (int mu=0; mu<n_opt; mu++)
    {
        Floats line;
        for (int nu=0; nu<n_opt; nu++)
            line.push_back(H(mu,nu));
        Hessian.push_back(line);
    }
    return Hessian;
}

  ParticlesTemp GaussianProcessInterpolation::get_Hessian_particles()
{
    ParticlesTemp ret;
    ParticlesTemp ret1 = mean_function_->get_input_particles();
    for (unsigned i=0; i<ret1.size(); i++)
    {
        if (Scale(ret1[i]).get_nuisance_is_optimized())
            ret.push_back(ret1[i]);
    }
    if (Scale(sigma_).get_nuisance_is_optimized())
        ret.push_back(sigma_);
    ret1 = covariance_function_->get_input_particles();
    for (unsigned i=0; i<ret1.size(); i++)
    {
        if (Scale(ret1[i]).get_nuisance_is_optimized())
            ret.push_back(ret1[i]);
    }
    return ret;
}

  void GaussianProcessInterpolation::do_show(std::ostream &out) const
{
    out << "Interpolation via gaussian process" << std::endl;
}

IMPISD_END_NAMESPACE
