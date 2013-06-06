/**
 *  \file IMP/isd/GaussianProcessInterpolationRestraintSparse.h
 *  \brief Normal distribution of Function
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_RESTRAINT_SPARSE_H
#define IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_RESTRAINT_SPARSE_H

#include <IMP/isd/isd_config.h>

#ifdef IMP_ISD_USE_CHOLMOD

#include <IMP/macros.h>
#include <boost/scoped_ptr.hpp>
#include <IMP/isd/univariate_functions.h>
#include <IMP/isd/bivariate_functions.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/isd/GaussianProcessInterpolationSparse.h>
#include <IMP/isd/MultivariateFNormalSufficientSparse.h>
#include <IMP/internal/OwnerPointer.h>
#include <Eigen/Dense>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>
#include <unsupported/Eigen/CholmodSupport>
#include <ufsparse/cholmod.h>



IMPISD_BEGIN_NAMESPACE
#ifndef SWIG
using Eigen::SparseMatrix;
using Eigen::MatrixXd;
using Eigen::VectorXd;
#endif

//! gaussian process restraint
/* the restraint is a multivariate normal distribution on the vector of
* observations with mean and standard deviation given by the posterior of the
* gaussian process.
*/
class IMPISDEXPORT GaussianProcessInterpolationRestraintSparse
      : public ISDRestraint
{
   private:
        // checks and makes necessary updates
        void update_mean_and_covariance();

   private:
        base::Pointer<GaussianProcessInterpolationSparse> gpi_;
        IMP::internal::OwnerPointer<MultivariateFNormalSufficientSparse> mvn_;
        //number of observation points
        unsigned M_;
        cholmod_common *c_;

   public:
        // this is a restraint on other restraints. It first constructs the
        // necessary vectors from GaussianProcessInterpolation, then creates a
        // multivariate normal distribution around it. Upon evaluation, it
        // checks if parameters have changed, reconstructs the matrix if
        // necessary, changes the DA weight and passes it to the functions.
        GaussianProcessInterpolationRestraintSparse(
                GaussianProcessInterpolationSparse *gpi);

        double get_probability() const
        {
            const_cast<GaussianProcessInterpolationRestraintSparse*>(this)->
                update_mean_and_covariance();
            return mvn_->density();
        }

        IMP_RESTRAINT(GaussianProcessInterpolationRestraintSparse);

};

IMPISD_END_NAMESPACE

#endif /* IMP_ISD_USE_CHOLMOD */

#endif  /* IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_RESTRAINT_SPARSE_H */
