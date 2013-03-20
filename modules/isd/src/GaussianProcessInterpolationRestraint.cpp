/**
 *  \file GaussianProcessInterpolationRestraint.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/GaussianProcessInterpolationRestraint.h>
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/jama_lu.h>
#include <boost/scoped_ptr.hpp>

IMPISD_BEGIN_NAMESPACE

GaussianProcessInterpolationRestraint::GaussianProcessInterpolationRestraint(
        GaussianProcessInterpolation *gpi) :
    ISDRestraint("GaussianProcessInterpolationRestraint %1%"), gpi_(gpi)
{
    //O(M^2)
    //number of observation points
    IMP_LOG_TERSE( "GPIR: init" << std::endl);
    M_ = gpi_->M_;
    // build multivariate normal with
    // mean : prior mean
    // covariance : prior covariance
    // observed at : the original observations
    IMP_LOG_TERSE( "GPIR: multivariate normal()" << std::endl);
    //args are: sample mean, jacobian, true mean,
    // nobs, sample variance, true variance
    mvn_ = new MultivariateFNormalSufficient(
            gpi_->get_I(), 1.0, gpi_->get_m(),
            1, Eigen::MatrixXd::Zero(M_,M_), gpi_->get_Omega());
    mvn_->set_use_cg(false,0.0);
    IMP_LOG_TERSE( "GPIR: done init" << std::endl);
}

void GaussianProcessInterpolationRestraint::set_model(Model *m)
{
    if (m) {
        IMP_LOG_TERSE( "GPIR: registering the model and scorestate"<<std::endl);
        Model *m = gpi_->sigma_->get_model();
        ss_ = new GaussianProcessInterpolationScoreState(this);
        m->add_score_state(ss_);
    } else {
        if (ss_) {
            if (ss_->get_is_part_of_model())
            {
                Model *m = ss_->get_model();
                m->remove_score_state(ss_);
                ss_=nullptr;
            }
        }
    }
    Restraint::set_model(m);
}

ModelObjectsTemp GaussianProcessInterpolationRestraint::do_get_inputs() const {
  // call the existing implementation
  ModelObjectsTemp ret;
  ret+=gpi_->get_input_particles();
  ret+=gpi_->get_input_containers();
  // add the score state
  ret.push_back(ss_);
  return ret;
}

double GaussianProcessInterpolationRestraint::unprotected_evaluate(
        DerivativeAccumulator *accum) const
{
    //the functions are all up-to-date since
    //the ScoreState has taken care of this

    if (accum) // O(M) if unchanged, O(M^2) if mean changed, else O(M^3)
    {
        //derivatives for mean particles
        VectorXd dmv = mvn_->evaluate_derivative_FM();
        unsigned npart=gpi_->get_number_of_m_particles();
        //std::cout << "derivs: ";
        for (unsigned i=0; i<npart; i++)
        {
            if (!gpi_->get_m_particle_is_optimized(i)) continue;
            VectorXd dmean = gpi_->get_m_derivative(i);
            double tmp = dmv.transpose()*dmean;
            //std::cout << i << " = " << tmp << " ";
            gpi_->add_to_m_particle_derivative(i, tmp, *accum);
        }
        //derivatives for covariance particles
        MatrixXd dmvS = mvn_->evaluate_derivative_Sigma();
        npart=gpi_->get_number_of_Omega_particles();
        for (unsigned i=0; i<npart; i++)
        {
            if (!gpi_->get_Omega_particle_is_optimized(i)) continue;
            MatrixXd dcov = gpi_->get_Omega_derivative(i);
            double tmp = (dmvS.transpose()*dcov).trace();
            //std::cout << i+2 << " = " << tmp << " ";
            gpi_->add_to_Omega_particle_derivative(i, tmp, *accum);
        }
        //std::cout << std::endl;
    }
    double ene = mvn_->evaluate(); //O(M^3) if Omega has changed else O(M)

    return ene;
}

double
GaussianProcessInterpolationRestraint::get_minus_log_normalization() const
{
    ss_->do_before_evaluate();
    return mvn_->get_minus_log_normalization();
}

double GaussianProcessInterpolationRestraint::get_minus_exponent() const
{
    ss_->do_before_evaluate();
    return mvn_->get_minus_exponent();
}


MatrixXd GaussianProcessInterpolationRestraint::get_hessian() const
{
    //update everything
    ss_->do_before_evaluate();
    //get how many and which particles are optimized
    unsigned mnum = gpi_->get_number_of_m_particles();
    std::vector<bool> mopt;
    unsigned mnum_opt = 0;
    for (unsigned i=0; i<mnum; i++)
    {
        mopt.push_back(gpi_->get_m_particle_is_optimized(i));
        if (mopt.back()) mnum_opt++;
    }
    unsigned Onum = gpi_->get_number_of_Omega_particles();
    std::vector<bool> Oopt;
    unsigned Onum_opt = 0;
    for (unsigned i=0; i<Onum; i++)
    {
        Oopt.push_back(gpi_->get_Omega_particle_is_optimized(i));
        if (Oopt.back()) Onum_opt++;
    }
    unsigned num_opt = mnum_opt + Onum_opt;
    // Only upper corner of hessian will be computed
    MatrixXd Hessian(MatrixXd::Zero(num_opt,num_opt));

    //d2E/(dm_k dm_l) * dm^k/dTheta_i dm^l/dTheta_j
    MatrixXd dmdm = mvn_->evaluate_second_derivative_FM_FM();
    std::vector<VectorXd> funcm;
    for (unsigned i=0; i<mnum; i++)
        if (mopt[i])
            funcm.push_back(gpi_->get_m_derivative(i));
    //     dm_k/dTheta_i = 0 if i is a covariance particle
    //     only fill upper triangle e.g. j>=i
    for (unsigned i=0; i<mnum_opt; ++i){
        VectorXd tmp(funcm[i].transpose()*dmdm);
        for (unsigned j=i; j<mnum_opt; ++j)
            Hessian(i,j) += tmp.transpose()*funcm[j];
    }
    dmdm.resize(0,0); // free up some space

    //d2E/(dOm_kl dOm_mn) * dOm_kl/dTheta_i * dOm_mn/dTheta_j
    std::vector<std::vector<MatrixXd> > dodo;
    //  get values
    for (unsigned m=0; m<M_; ++m) //row of second matrix
    {
        std::vector<MatrixXd> tmp;
        for (unsigned n=0; n<M_; ++n) //column of second matrix
            tmp.push_back(mvn_->evaluate_second_derivative_Sigma_Sigma(m,n));
        dodo.push_back(tmp);
    }
    std::vector< MatrixXd > funcO;
    for (unsigned i=0; i<Onum; ++i)
        if (Oopt[i])
            funcO.push_back(gpi_->get_Omega_derivative(i));
    //  compute contribution
    for (unsigned i=mnum_opt; i<num_opt; ++i){
        MatrixXd tmp(M_,M_);
        for (unsigned m=0; m<M_; ++m)
            for (unsigned n=0; n<M_; ++n)
                    tmp(m,n) =
                        (dodo[m][n].transpose()*funcO[i-mnum_opt]).trace();
        for (unsigned j=i; j<num_opt; ++j)
            Hessian(i,j) +=
                (tmp*funcO[j-mnum_opt]).trace();
    }
    for (unsigned i=0; i < dodo.size(); ++i)
        for (unsigned j=0; j < dodo[i].size(); ++j)
            dodo[i][j].resize(0,0);

    //d2E/(dm_k dOm_lm) * (  dm^k/dTheta_j dOm^lm/dTheta_i
    //                     + dm^k/dTheta_i dOm^lm/dTheta_j)
    //when i<=j, simplifies to i mean particles and j covariance particles:
    //d2E/(dm_k dOm_lm) * (  dm^k/dTheta_i dOm^lm/dTheta_j)
    //
    //get values
    std::vector<MatrixXd> dmdo;
    for (unsigned k=0; k<M_; k++)
        dmdo.push_back(mvn_->evaluate_second_derivative_FM_Sigma(k));
    //compute hessian
    for (unsigned j=mnum_opt; j<num_opt; ++j)
    {
        VectorXd tmp(M_);
        for (unsigned k=0; k<M_; ++k)
                tmp(k) = (dmdo[k].transpose()*funcO[j-mnum_opt]).trace();
        for (unsigned i=0; i<mnum_opt; ++i)
            Hessian(i,j) += funcm[i].transpose()*tmp;
    }
    //deallocate both dmdo and all function derivatives
    for (unsigned k=0; k<dmdo.size(); ++k)
        dmdo[k].resize(0,0);
    for (unsigned i=0; i<funcm.size(); ++i)
        funcm[i].resize(0);
    for (unsigned i=0; i<funcO.size(); ++i)
            funcO[i].resize(0,0);

    // dE/dm_k * d2m^k/(dTheta_i dTheta_j)
    VectorXd dem(mvn_->evaluate_derivative_FM());
    for (unsigned i=0, iopt=0; i<mnum; i++){
        if (!mopt[i]) continue;
        for (unsigned j=i, jopt=iopt; j<mnum; j++){
            if (!mopt[j]) continue;
            Hessian(iopt,jopt) +=
                dem.transpose()*gpi_->get_m_second_derivative(i,j);
            jopt++;
        }
        iopt++;
    }

    dem.resize(0);

    // dE/dOm_kl * d2Om^kl/(dTheta_i dTheta_j)
    MatrixXd dOm(mvn_->evaluate_derivative_Sigma());
    for (unsigned i=mnum, iopt=mnum_opt; i<mnum+Onum; i++){
        if (!Oopt[i-mnum]) continue;
        for (unsigned j=i, jopt=iopt; j<mnum+Onum; j++){
            if (!Oopt[j-mnum]) continue;
            Hessian(iopt,jopt) += (dOm.transpose()
                    *gpi_->get_Omega_second_derivative(i-mnum,j-mnum)).trace();
            jopt++;
        }
        iopt++;
    }
    dOm.resize(0,0);

    //return hessian as full matrix
    for (unsigned i=0; i<num_opt; ++i)
        for (unsigned j=i+1; j<num_opt; ++j)
            Hessian(j,i) = Hessian(i,j);
    //return Hessian.selfadjointView<Eigen::Upper>();
    return Hessian;
}

FloatsList GaussianProcessInterpolationRestraint::get_hessian(bool) const
{
    MatrixXd tmp(get_hessian());
    FloatsList ret;
    for (unsigned i=0; i<tmp.rows(); ++i){
        Floats buf;
        for (unsigned j=0; j< tmp.cols(); ++j)
            buf.push_back(tmp(i,j));
        ret.push_back(buf);
    }
    return ret;
}

double GaussianProcessInterpolationRestraint::get_logdet_hessian() const
{
    //compute ldlt
    Eigen::LDLT<MatrixXd, Eigen::Upper> ldlt(get_hessian());
    if (!ldlt.isPositive())
        IMP_THROW("Hessian matrix is not positive definite!", ModelException);
    return ldlt.vectorD().array().abs().log().sum();
}

void GaussianProcessInterpolationScoreState::do_before_evaluate()
{
    IMP_LOG_TERSE( "GPISS: do_before_evaluate()" << std::endl);
    GaussianProcessInterpolation *gpi_;
    gpi_ = gpir_->gpi_;
    MultivariateFNormalSufficient *mvn_;
    mvn_ = gpir_->mvn_;
    //
    gpi_->update_flags_covariance(); //O(1)
    gpi_->update_flags_mean(); //O(1)
    if (!(gpi_->flag_m_gpir_))  // gpi says that our m is not up to date
    {
        mvn_->set_FM(gpi_->get_m()); // O(M_)
        gpi_->flag_m_gpir_ = true;
        IMP_LOG_TERSE( " updated mean");
    }
    if (!(gpi_->flag_Omega_gpir_))
    {
        mvn_->set_Sigma(gpi_->get_Omega()); // O(M^2)
        gpi_->flag_Omega_gpir_ = true;
        IMP_LOG_TERSE( " updated covariance");
    }
    IMP_LOG_TERSE( std::endl);
    /*ParticlesTemp tmp(gpir_->get_input_particles());
    std::cout << "values: ";
    for (unsigned i=0; i<tmp.size(); i++)
    {
        std::cout << i << " = " << Scale(tmp[i]).get_nuisance() << " ";
    }
    std::cout <<std::endl;*/
}

void GaussianProcessInterpolationScoreState::do_after_evaluate(
        DerivativeAccumulator *) {
}

ContainersTemp
GaussianProcessInterpolationScoreState::get_input_containers() const {
  return gpir_->gpi_->get_input_containers();
}
ContainersTemp
GaussianProcessInterpolationScoreState::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp
GaussianProcessInterpolationScoreState::get_input_particles() const {
  //gpir needs to update internal values computed from particles
  return gpir_->gpi_->get_input_particles();
}
ParticlesTemp
GaussianProcessInterpolationScoreState::get_output_particles() const {
  //gpir does not change particles' attributes.
  return ParticlesTemp();
}

void GaussianProcessInterpolationScoreState::do_show(std::ostream &out) const
{
    out << "GPI score state" << std::endl;
}

IMPISD_END_NAMESPACE
