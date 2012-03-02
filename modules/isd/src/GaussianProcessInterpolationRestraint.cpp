/**
 *  \file GaussianProcessInterpolationRestraint.cpp
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
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
            if (ss_->get_has_model())
            {
                Model *m = ss_->get_model();
                m->remove_score_state(ss_);
                ss_=nullptr;
            }
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
    double ene = mvn_->evaluate();

    return ene;
}

ParticlesTemp
GaussianProcessInterpolationRestraint::get_input_particles() const
{
    return gpi_->get_input_particles();
}

ContainersTemp GaussianProcessInterpolationRestraint::get_input_containers()
    const
{
    return gpi_->get_input_containers();
}

void GaussianProcessInterpolationRestraint::do_show(std::ostream& out) const
{
    out << "GaussianProcessInterpolationRestraint on "
        << get_input_particles().size() << " particles" << std::endl;
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
  return gpir_->get_input_containers();
}
ContainersTemp
GaussianProcessInterpolationScoreState::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp
GaussianProcessInterpolationScoreState::get_input_particles() const {
  //gpir needs to update internal values computed from particles
  return gpir_->get_input_particles();
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
