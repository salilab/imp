/**
 *  \file GaussianProcessInterpolation.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_RESTRAINT_H
#define IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_RESTRAINT_H

#include "isd_config.h"
#include <IMP/macros.h>
#include <IMP/algebra/internal/tnt_array1d.h>
#include <IMP/algebra/internal/tnt_array1d_utils.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/algebra/internal/jama_lu.h>
#include <boost/scoped_ptr.hpp>
#include <IMP/isd/functions.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/isd/GaussianProcessInterpolation.h>
#include <IMP/isd/MultivariateFNormalSufficient.h>
#include <IMP/internal/OwnerPointer.h>


IMPISD_BEGIN_NAMESPACE
#ifndef SWIG
using IMP::algebra::internal::TNT::Array1D;
using IMP::algebra::internal::TNT::Array2D;
#endif

//! gaussian process restraint
/* the restraint is a multivariate normal distribution on the vector of
* observations with mean and standard deviation given by the posterior of the
* gaussian process.
*/
class IMPISDEXPORT GaussianProcessInterpolationRestraint : public ISDRestraint
{
   private:
        //compute posterior mean vector
        void compute_mean_vector();
        //compute posterior covariance matrix
        void compute_covariance_matrix();
        // checks and makes necessary updates
        void update_mean_and_covariance();

        // trans(W)(W+S)^{-1} needed for routines below
        void compute_tWWS();
        
        //derivative of posterior mean vector wrt prior mean vector entry i
        Array1D<double> compute_dmdm(unsigned i);
        //derivative of posterior mean vector wrt prior covariance matrix
        //entry (i,j)
        Array1D<double> compute_dmdW(unsigned i, unsigned j);
        //derivative of posterior covariance matrix wrt prior covariance
        //matrix
        Array2D<double> compute_dCdW(unsigned i, unsigned j);

    private:
        IMP::Pointer<GaussianProcessInterpolation> gpi_;
        IMP::internal::OwnerPointer<MultivariateFNormalSufficient> mvn_;
        //posterior mean vector
        Array1D<double> mean_;
        //posterior covariance matrix
        Array2D<double> Cov_;
        //number of observation points
        unsigned M_;
        // trans(W)(W+S)^{-1}
        Array2D<double> tWWS_;
public:
        // this is a restraint on other restraints. It first constructs the
        // necessary vectors from GaussianProcessInterpolation, then creates a
        // multivariate normal distribution around it. Upon evaluation, it
        // checks if parameters have changed, reconstructs the matrix if
        // necessary, changes the DA weight and passes it to the functions.
        GaussianProcessInterpolationRestraint(
                GaussianProcessInterpolation *gpi);

        double get_probability() const
        {
            const_cast<GaussianProcessInterpolationRestraint*>(this)->
                update_mean_and_covariance();
            return mvn_->density();
        }

        IMP_RESTRAINT(GaussianProcessInterpolationRestraint);
    
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_RESTRAINT_H */
