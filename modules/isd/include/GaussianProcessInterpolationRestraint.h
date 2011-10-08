/**
 *  \file GaussianProcessInterpolation.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_RESTRAINT_H
#define IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_RESTRAINT_H

#include "isd_config.h"
#include <IMP/macros.h>
#include <boost/scoped_ptr.hpp>
#include <IMP/isd/functions.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/isd/GaussianProcessInterpolation.h>
#include <IMP/isd/MultivariateFNormalSufficient.h>
#include <IMP/internal/OwnerPointer.h>
#include <Eigen/Dense>
#include <Eigen/Cholesky>


IMPISD_BEGIN_NAMESPACE
#ifndef SWIG
using Eigen::MatrixXd;
using Eigen::VectorXd;
#endif

//! gaussian process restraint
/* the restraint is a multivariate normal distribution on the vector of
* observations with mean and standard deviation given by the posterior of the
* gaussian process.
*/
class IMPISDEXPORT GaussianProcessInterpolationRestraint : public ISDRestraint
{
   private:
        // checks and makes necessary updates
        void update_mean_and_covariance();

   private:
        IMP::Pointer<GaussianProcessInterpolation> gpi_;
        IMP::internal::OwnerPointer<MultivariateFNormalSufficient> mvn_;
        //number of observation points
        unsigned M_;

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

        void stats() const
        {
            return mvn_->stats();
        }

        IMP_RESTRAINT(GaussianProcessInterpolationRestraint);
    
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_RESTRAINT_H */
