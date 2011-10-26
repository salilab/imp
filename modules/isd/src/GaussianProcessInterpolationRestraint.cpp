/**
 *  \file GaussianProcessInterpolationRestraint.cpp
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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
        GaussianProcessInterpolation *gpi) : gpi_(gpi)
{
    //number of observation points
    IMP_LOG(TERSE, "GPIR: init" << std::endl);
    M_ = gpi_->M_;
    // build multivariate normal with
    // mean : prior mean
    // covariance : prior covariance
    // observed at : the original observations
    IMP_LOG(TERSE, "GPIR: multivariate normal()" << std::endl);
    //args are: sample mean, jacobian, true mean,
    // nobs, sample variance, true variance
    mvn_ = new MultivariateFNormalSufficient(
            gpi_->get_I(), 1.0, gpi_->get_m(),
            1, Eigen::MatrixXd::Zero(M_,M_), gpi_->get_Omega());
    mvn_->set_use_cg(false,0.0);
    IMP_LOG(TERSE, "GPIR: done init" << std::endl);
}

void GaussianProcessInterpolationRestraint::set_model(Model *m)
{
    if (m) {
        IMP_LOG(TERSE, "GPIR: registering the model and scorestate"<<std::endl);
        Model *m = gpi_->sigma_->get_model();
        ss_ = new GaussianProcessInterpolationScoreState(this);
        m->add_score_state(ss_);
    } else {
        if (ss_) {
            Model *m = gpi_->sigma_->get_model();
            m->remove_score_state(ss_);
            ss_=nullptr;
        }
    }
    Restraint::set_model(m);
}

double GaussianProcessInterpolationRestraint::unprotected_evaluate(
        DerivativeAccumulator *accum) const
{
    //the functions are all up-to-date since
    //the ScoreState has taken care of this

    if (accum)
    {
        //derivatives for mean particles
        VectorXd dmv = mvn_->evaluate_derivative_FM();
        unsigned npart=gpi_->get_number_of_optimized_m_particles();
        for (unsigned i=0; i<npart; i++)
        {
            VectorXd dmean = gpi_->get_m_derivative(i);
            double tmp = dmv.transpose()*dmean;
            gpi_->add_to_m_particle_derivative(i, tmp, *accum);
        }
        //derivatives for covariance particles
        MatrixXd dmvS = mvn_->evaluate_derivative_Sigma();
        npart=gpi_->get_number_of_optimized_Omega_particles();
        for (unsigned i=0; i<npart; i++)
        {
            MatrixXd dcov = gpi_->get_Omega_derivative(i);
            double tmp = (dmvS.transpose()*dcov).trace();
            gpi_->add_to_Omega_particle_derivative(i, tmp, *accum);
        }
    }
    double ene = mvn_->evaluate();

    return ene;
}

ParticlesTemp
GaussianProcessInterpolationRestraint::get_input_particles() const
{
    ParticlesTemp ret;
    ParticlesTemp ret1 = gpi_->mean_function_->get_input_particles();
    ret.insert(ret.end(),ret1.begin(),ret1.end());
    ret.push_back(gpi_->sigma_);
    ParticlesTemp ret2 = gpi_->covariance_function_->get_input_particles();
    ret.insert(ret.end(),ret2.begin(),ret2.end());
    return ret;
}

ContainersTemp GaussianProcessInterpolationRestraint::get_input_containers()
    const
{
    ContainersTemp ret;
    ContainersTemp ret1 = gpi_->mean_function_->get_input_containers();
    ret.insert(ret.end(),ret1.begin(),ret1.end());
    ContainersTemp ret2 = gpi_->covariance_function_->get_input_containers();
    ret.insert(ret.end(),ret2.begin(),ret2.end());
    return ret;
}

void GaussianProcessInterpolationRestraint::do_show(std::ostream& out) const
{
    out << "GaussianProcessInterpolationRestraint on "
        << get_input_particles().size() << " particles" << std::endl;
}




void GaussianProcessInterpolationScoreState::do_before_evaluate()
{
    IMP_LOG(TERSE, "GPISS: do_before_evaluate()" << std::endl);
    GaussianProcessInterpolation *gpi_;
    gpi_ = gpir_->gpi_;
    MultivariateFNormalSufficient *mvn_;
    mvn_ = gpir_->mvn_;
    //
    gpi_->update_flags_covariance();
    gpi_->update_flags_mean();
    if (!(gpi_->flag_m_gpir_))  // gpi says that our m is not up to date
    {
        mvn_->set_FM(gpi_->get_m());
        gpi_->flag_m_gpir_ = true;
        IMP_LOG(TERSE, " updated mean");
    }
    if (!(gpi_->flag_Omega_gpir_))
    {
        mvn_->set_Sigma(gpi_->get_Omega());
        gpi_->flag_Omega_gpir_ = true;
        IMP_LOG(TERSE, " updated covariance");
    }
    IMP_LOG(TERSE, std::endl);
}

void GaussianProcessInterpolationScoreState::do_after_evaluate(
        DerivativeAccumulator *) {
}

ContainersTemp
GaussianProcessInterpolationScoreState::get_input_containers() const {
  return gpir_->get_input_containers();
}
ContainersTemp
GaussianProcessInterpolationScoreState::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp
GaussianProcessInterpolationScoreState::get_input_particles() const {
  return gpir_->get_input_particles();
}
ParticlesTemp
GaussianProcessInterpolationScoreState::get_output_particles() const {
  return ParticlesTemp();
}

void GaussianProcessInterpolationScoreState::do_show(std::ostream &out) const
{
    out << "GPI score state" << std::endl;
}

IMPISD_END_NAMESPACE
