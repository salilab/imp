/**
 *  \file GaussianProcessInterpolationRestraintSparse.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/GaussianProcessInterpolationRestraintSparse.h>

#ifdef IMP_ISD_USE_CHOLMOD

#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/jama_lu.h>
#include <boost/scoped_ptr.hpp>

IMPISD_BEGIN_NAMESPACE

GaussianProcessInterpolationRestraintSparse::
    GaussianProcessInterpolationRestraintSparse(
        GaussianProcessInterpolationSparse *gpi) : gpi_(gpi)
{
    //number of observation points
    IMP_LOG_TERSE( "GPIR: init" << std::endl);
    M_ = gpi_->n_obs_.size();
    //number of repetitions
    int N=gpi_->n_obs_[0];
    //check if the number of repetitions is the same for every point
    IMP_IF_CHECK(USAGE) {
        for (unsigned i=1; i<M_; i++)
            IMP_USAGE_CHECK(N == gpi_->n_obs_[i],
                "must have the same number of repetitions for each point!");
    }
    // build multivariate normal with
    // mean : prior mean
    // covariance : prior covariance
    // observed at : the original observations
    IMP_LOG_TERSE( "GPIR: multivariate normal()" << std::endl);
    //args are: sample mean, jacobian, true mean,
    // nobs, sample variance, true variance
    c_ = gpi_->get_cholmod_common();
    mvn_ = new MultivariateFNormalSufficientSparse(gpi_->get_I(), 1.0,
            gpi_->get_m(), N, gpi_->get_S(), gpi_->get_W(), c_);
    IMP_LOG_TERSE( "GPIR: done init" << std::endl);
}

void GaussianProcessInterpolationRestraintSparse::update_mean_and_covariance()
{
    IMP_LOG_TERSE( "GPIR: update_mean_and_covariance" << std::endl);
    gpi_->update_flags_covariance();
    gpi_->update_flags_mean();
    if (!(gpi_->flag_m_gpir_))  // gpi says that our m is not up to date
    {
        mvn_->set_FM(gpi_->get_m());
        gpi_->flag_m_gpir_ = true;
        IMP_LOG_TERSE( " updated mean");
    }
    if (!(gpi_->flag_W_gpir_))
    {
        mvn_->set_Sigma(gpi_->get_W());
        gpi_->flag_W_gpir_ = true;
        IMP_LOG_TERSE( " updated covariance");
    }
    IMP_LOG_TERSE( std::endl);
}

double GaussianProcessInterpolationRestraintSparse::unprotected_evaluate(
                         DerivativeAccumulator *accum) const
{
    //check if the functions have changed
    const_cast<GaussianProcessInterpolationRestraintSparse*>(this)->
        update_mean_and_covariance();

    double ene = mvn_->evaluate();

    if (accum)
    {
        cholmod_dense *dmv = mvn_->evaluate_derivative_FM();
        if (dmv->xtype != CHOLMOD_REAL)
            IMP_THROW("matrix type is not real, update code here first",
                    ModelException);
        double *dmvx=(double*) dmv->x;
        //derivatives for mean particles
        for (size_t i=0; i<M_; i++)
        {
            DerivativeAccumulator a(*accum, dmvx[i]);
            gpi_->mean_function_->add_to_derivatives(gpi_->x_[i], a);
        }
        cholmod_free_dense(&dmv, c_);

        //derivatives for covariance particles
        cholmod_sparse *tmp(mvn_->evaluate_derivative_Sigma());
        cholmod_triplet *dmvS = cholmod_sparse_to_triplet(tmp, c_);
        cholmod_free_sparse(&tmp, c_);
        if ((dmvS->itype != CHOLMOD_INT) && (dmvS->xtype != CHOLMOD_REAL))
            IMP_THROW("matrix type is not real or coefficients are not int!",
                    ModelException);
        int *dmvi=(int*) dmvS->i;
        int *dmvj=(int*) dmvS->j;
        dmvx=(double*) dmvS->x;
        for (size_t p=0; p<dmvS->nzmax; ++p)
        {
            int i=dmvi[p];
            int j=dmvj[p];
            double val=dmvx[p];
            DerivativeAccumulator a(*accum, val);
            gpi_->covariance_function_->add_to_derivatives(
                    gpi_->x_[i],gpi_->x_[j], a);
        }
        cholmod_free_triplet(&dmvS,c_);
    }
    return ene;
}

ParticlesTemp
GaussianProcessInterpolationRestraintSparse::get_input_particles() const
{
    ParticlesTemp ret;
    ParticlesTemp ret1 = gpi_->mean_function_->get_input_particles();
    ret.insert(ret.end(),ret1.begin(),ret1.end());
    ParticlesTemp ret2 = gpi_->covariance_function_->get_input_particles();
    ret.insert(ret.end(),ret2.begin(),ret2.end());
    return ret;
}

ContainersTemp
GaussianProcessInterpolationRestraintSparse::get_input_containers()
    const
{
    ContainersTemp ret;
    ContainersTemp ret1 = gpi_->mean_function_->get_input_containers();
    ret.insert(ret.end(),ret1.begin(),ret1.end());
    ContainersTemp ret2 = gpi_->covariance_function_->get_input_containers();
    ret.insert(ret.end(),ret2.begin(),ret2.end());
    return ret;
}

void GaussianProcessInterpolationRestraintSparse::do_show(
                                           std::ostream& out) const
{
    out << "GaussianProcessInterpolationRestraintSparse on "
        << get_input_particles().size() << " particles" << std::endl;
}

IMPISD_END_NAMESPACE

#endif /* IMP_ISD_USE_CHOLMOD */
