/**
 *  \file GaussianProcessInterpolationRestraint.cpp  
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/GaussianProcessInterpolationRestraint.h>
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/jama_lu.h>
#include <boost/scoped_ptr.hpp>

IMPISD_BEGIN_NAMESPACE

using IMP::algebra::internal::TNT::Array1D;
using IMP::algebra::internal::TNT::Array2D;

GaussianProcessInterpolationRestraint::GaussianProcessInterpolationRestraint(
        GaussianProcessInterpolation *gpi) : gpi_(gpi)
{
    //number of observation points
    IMP_LOG(TERSE, "GPIR: init" << std::endl);
    M_ = gpi_->n_obs_.size();
    // build multivariate normal with 
    // mean : posterior mean
    // covariance : posterior covariance
    // observed at : the original observations
    IMP_LOG(TERSE, "GPIR: compute_mean_vector()" << std::endl);
    compute_mean_vector();
    IMP_LOG(TERSE, "GPIR: compute_covariance_matrix()" << std::endl);
    compute_covariance_matrix();
    IMP_LOG(TERSE, "GPIR: multivariate normal()" << std::endl);
    Array2D<double> W(M_,M_,0.0); //data correlation assumed not to be reported
    mvn_ = new MultivariateFNormalSufficient(gpi_->I_, 1.0, mean_, 1, W, Cov_);
    IMP_LOG(TERSE, "GPIR: done init" << std::endl);
}

void GaussianProcessInterpolationRestraint::compute_mean_vector()
{
    mean_ = Array1D<double> (M_);
    for (unsigned i=0; i<M_; i++)
    {
        mean_[i] = gpi_->get_posterior_mean(gpi_->x_[i]);
        IMP_LOG(TERSE, "GPIR: mean_["<<i<<"]= "<<mean_[i]<< std::endl);
    }
    IMP_LOG(TERSE, std::endl);
}

void GaussianProcessInterpolationRestraint::compute_covariance_matrix()
{
    Cov_ = Array2D<double> (M_,M_);
    for (unsigned i=0; i<M_; i++)
    {
        Cov_[i][i] = gpi_->get_posterior_covariance(gpi_->x_[i],gpi_->x_[i]);
        IMP_LOG(TERSE, "GPIR: Cov_["<<i<<"]["<<i<<"]= "
                                    <<Cov_[i][i]<< std::endl);
        for (unsigned j=i+1; j<M_; j++)
        {
            Cov_[i][j] = gpi_->get_posterior_covariance(gpi_->x_[i],gpi_->x_[j]);
            Cov_[j][i] = Cov_[i][j];
            IMP_LOG(TERSE, "GPIR: Cov_["<<i<<"]["<<j<<"]= "
                    <<Cov_[i][j]<< std::endl);
        }
    }
}

void GaussianProcessInterpolationRestraint::update_mean_and_covariance()
{
    bool cov = gpi_->covariance_function_->has_changed();
    bool mean = gpi_->mean_function_->has_changed();
    if (mean || cov)
    {
        compute_mean_vector();
        mvn_->set_FM(mean_);
    }
    if (cov)
    {
        compute_covariance_matrix();
        mvn_->set_Sigma(Cov_);
    }
}

/* trans(W)(W+S)^{-1}
 * O(M^3)
 */
void GaussianProcessInterpolationRestraint::compute_tWWS()
{
    tWWS_ = Array2D<double> (M_,M_);  
    for (unsigned i=0; i<M_; i++)
    {
        for (unsigned j=0; j<M_; j++)
        {
            tWWS_[i][j] = 0;
            for (unsigned k=0; k<M_; k++)
            {
                tWWS_[i][j] += gpi_->W_[k][i]*gpi_->WS_[k][j];
            }
            //std::cout << "tWWS_["<<i<<"]["<<j<<"]= "<<tWWS_[i][j]<<std::endl;
        }
    }
}

/* d(mhat)/d(m_i) = (Id - trans(W)(W+S)^{-1})J_i
* with J_i the single-entry vector.
* Assumes the functions and the matrices are in sync, which is the case when
* calling unprotected_evaluate
* O(M)
*/
Array1D<double> GaussianProcessInterpolationRestraint::compute_dmdm(unsigned i)
{
    Array1D<double> ret(M_);
    for (unsigned j=0; j<M_; j++)
    {
        ret[j] = (i==j) ? 1 : 0;
        ret[j] -= tWWS_[j][i];
    }
    //std::cout << "dmdm: "<< i << " " << ret[0] << " " << ret[1] << std::endl;
    return ret;
}

/*d(mhat)/dW_{i,j} = [Id - trans(W)(W+S)^{-1}]J^{ij}(W+S)^{-1}(I-m)
 * same conditions as dmdm.
 * O(M)
 */
Array1D<double> GaussianProcessInterpolationRestraint::compute_dmdW(unsigned i, unsigned j)
{
    //get nonzero coefficient of J^{ij}(W+S)^{-1}(I-m)
    double coef = gpi_->WSIm_[j];
    // call compute_dmdm since it's the same computation
    Array1D<double> ret(compute_dmdm(i));
    for (unsigned k=0; k<M_; k++) ret[k] *= coef;
    return ret;
}

/*d(Sigmahat)/dW_{i,j} = J^{ij} - trans(J^{ji}A) (Id - A) - J^{ji}A
 * where A=(W+S)^{-1} W
 * same conditions as dmdm.
 * O(M)
 */
Array2D<double> GaussianProcessInterpolationRestraint::compute_dCdW(unsigned i, unsigned j)
{
    Array2D<double> ret(M_,M_);
    for (unsigned k=0; k<M_; k++)
    {
        for (unsigned l=0; l<M_; l++)
        {
            ret[k][l] = tWWS_[k][i]*tWWS_[l][j];
            if (k==j) ret[k][l] -= tWWS_[l][i];
            if (l==j) ret[k][l] -= tWWS_[k][i];
            if (l==j && k==i) ret[k][l] += 1;
        }
    }
    return ret;
}

double GaussianProcessInterpolationRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
    //check if the functions have changed
    const_cast<GaussianProcessInterpolationRestraint*>(this)->
        update_mean_and_covariance();
    
    double ene = mvn_->evaluate();

    if (accum)
    {
        const_cast<GaussianProcessInterpolationRestraint*>(this)->
            compute_tWWS();
        Array1D<double> dmv = mvn_->evaluate_derivative_FM();
        //derivatives for mean particles
        for (unsigned i=0; i<M_; i++)
        {
            Array1D<double> dmdm = 
                const_cast<GaussianProcessInterpolationRestraint*>(this)->
                    compute_dmdm(i);
            double sum=0;
            for (unsigned j=0; j<M_; j++) sum += dmv[j]*dmdm[j];
            DerivativeAccumulator a(*accum, sum);
            gpi_->mean_function_->add_to_derivatives(gpi_->x_[i], a);
        }
        //derivatives for covariance particles
        Array2D<double> dmvS = mvn_->evaluate_derivative_Sigma();
        for (unsigned k=0; k<M_; k++)
        {
            for (unsigned l=0; l<M_; l++)
            {
                Array1D<double> dmdW = 
                    const_cast<GaussianProcessInterpolationRestraint*>(this)->
                        compute_dmdW(k,l);
                Array2D<double> dCdW = 
                    const_cast<GaussianProcessInterpolationRestraint*>(this)->
                        compute_dCdW(k,l);
                double partial_I=0;
                double partial_S=0;
                //std::cout << "dmvS["<<k<<"]["<<l<<"] = "
                //                <<dmvS[k][l] << std::endl;
                for (unsigned i=0; i<M_; i++)
                {
                    partial_I += dmv[i]*dmdW[i];
                    for (unsigned j=0; j<M_; j++)
                    {
                        partial_S += dmvS[i][j]*dCdW[i][j];
                        //std::cout << "   dCdW["<<i<<"]["<<j<<"] = "
                        //            <<dCdW[i][j] << std::endl;
                    }
                }
                DerivativeAccumulator a(*accum, partial_S+partial_I);
                gpi_->covariance_function_->add_to_derivatives(
                        gpi_->x_[k],gpi_->x_[l], a);
            }
        }
    }
    return ene;
}

ParticlesTemp GaussianProcessInterpolationRestraint::get_input_particles() const
{
    ParticlesTemp ret;
    ParticlesTemp ret1 = gpi_->mean_function_->get_input_particles();
    ret.insert(ret.end(),ret1.begin(),ret1.end());
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

IMPISD_END_NAMESPACE

