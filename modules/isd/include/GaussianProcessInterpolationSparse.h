/**
 *  \file IMP/isd/GaussianProcessInterpolationSparse.h
 *  \brief Normal distribution of Function
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_SPARSE_H
#define IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_SPARSE_H

#include <IMP/isd/isd_config.h>

#ifdef IMP_ISD_USE_CHOLMOD

#include <IMP/macros.h>
#include <boost/scoped_ptr.hpp>
#include <IMP/isd/univariate_functions.h>
#include <IMP/isd/bivariate_functions.h>
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

class GaussianProcessInterpolationRestraintSparse;

//! GaussianProcessInterpolationSparse
/** This class provides methods to perform bayesian interpolation/smoothing of
 * data using a gaussian process prior on the function to be interpolated.
 * It takes a dataset as input (via its sufficient statistics) along with prior
 * mean and covariance functions. It outputs the value of the posterior mean and
 * covariance functions at points requested by the user.
 */
class IMPISDEXPORT GaussianProcessInterpolationSparse : public base::Object
{
 public:
     /** Constructor for the gaussian process
      * \param(in) x : a list of coordinates in N-dimensional space
      *                corresponding to the abscissa of each observation
      * \param(in) sample_mean \f$I\f$ : vector of mean observations at each of
      *                                  the previously defined coordinates
      * \param(in) sample_std \f$s\f$ : vector of sample standard deviations
      * \param(in) mean_function \f$m\f$ : a pointer to the prior mean function
      *                                    to use.  Should be compatible with
      *                                    the size of x(i).
      * \param(in) covariance_function \f$w\f$: prior covariance function.
      * \param(in) cutoff : when to consider that W matrix entries are zero.
      *
      * Computes the necessary matrices and inverses when called.
      */
  GaussianProcessInterpolationSparse(FloatsList x,
                               Floats sample_mean,
                               Floats sample_std,
                               Ints n_obs,
                               UnivariateFunction *mean_function,
                               BivariateFunction *covariance_function,
                               double cutoff=1e-7);

  /** Get posterior mean and covariance functions, at the points requested
   * Posterior mean is defined as
   * \f[\hat{I}(x) = m(x)
   *        + {}^t\mathbf{w}(q)
   *        (\mathbf{W}+\mathbf{S})^{-1}
   *        (\mathbf{I}-\mathbf{m}) \f]
   * Posterior covariance
   * \f[\hat{\sigma}^2(x,x') =
   *    w(x,x') - {}^t\mathbf{w}(x)
   *              (\mathbf{W} + \mathbf{S})^{-1}
   *              \mathbf{w}(x') \f]
   * where \f$\mathbf{m}\f$ is the vector built by evaluating the prior mean
   * function at the observation points; \f$\mathbf{w}(x)\f$ is the vector of
   * covariances between each observation point and the current point;
   * \f$\mathbf{W}\f$ is the prior covariance matrix built by evaluating the
   * covariance function at each of the observations; \f$\mathbf{S}\f$ is the
   * diagonal covariance matrix built from sample_std and n_obs.
   *
   * Both functions will check if the mean or covariance functions have changed
   * since the last call, and will recompute
   * \f$(\mathbf{W} + \mathbf{S})^{-1}\f$ if necessary.
   */
  double get_posterior_mean(Floats x);
  double get_posterior_covariance(Floats x1,
                                  Floats x2);

  // call these if you called update() on the mean or covariance function.
  // it will force update any internal variables dependent on these functions.
  void force_mean_update();
  void force_covariance_update();

  friend class GaussianProcessInterpolationRestraintSparse;

  IMP_OBJECT_METHODS(GaussianProcessInterpolationSparse);
  /*IMP_OBJECT_INLINE(GaussianProcessInterpolationSparse,
          out << "GaussianProcessInterpolationSparse :"
          "learning from " << M_ << " "
          << N_ << "-dimensional observations" << std::endl,
          {
          cholmod_free_factor(&L_, c_);
          cholmod_free_dense(&WSIm_, c_);
          cholmod_free_sparse(&WS_,c_);
          cholmod_finish(c_);
          });*/

 protected:
  //returns updated data vector
  VectorXd get_I() const {return I_;}
  //returns updated prior mean vector
  VectorXd get_m();
  // returns updated prior covariance vector
  cholmod_sparse *get_wx_vector(Floats xval);
  //returns updated data covariance matrix
  SparseMatrix<double> get_S() const {return S_;}
  //returns updated prior covariance matrix
  SparseMatrix<double> get_W();
  //returns updated (W+S)^{-1}
  cholmod_sparse *get_WS();
  //returns updated cholesky factor
  cholmod_factor *get_L();
  //returns updated (W+S)^{-1}(I-m)
  cholmod_dense *get_WSIm();

 private:

  // ensures the mean/covariance function has updated parameters. Signals an
  // update by changing the state flags. Returns true if the function has
  // changed. This is used by GaussianProcessInterpolationSparseRestraint.
  void update_flags_mean();
  void update_flags_covariance();

  // compute prior covariance matrix
  void compute_W();
  // compute \f$(\mathbf{W} + \mathbf{S})^{-1}\f$ (if necessary).
  void compute_WS();
  // compute (W+S)^{-1} (I-m)
  void compute_WSIm();

  // compute mean observations
  void compute_I(Floats mean);
  // compute diagonal covariance matrix of observations
  void compute_S(Floats std, Ints n);
  // compute prior mean vector
  void compute_m();

  // for GPIR.
  cholmod_common *get_cholmod_common() { return c_; }

 private:
    unsigned N_; // number of dimensions of the abscissa
    unsigned M_; // number of observations to learn from
    FloatsList x_; // abscissa
    Ints n_obs_; // number of observations
    // pointer to the prior mean function
    IMP::internal::OwnerPointer<UnivariateFunction> mean_function_;
    // pointer to the prior covariance function
    IMP::internal::OwnerPointer<BivariateFunction> covariance_function_;
    VectorXd I_,m_;
    SparseMatrix<double> S_,W_,wx_;
    cholmod_sparse cwx_; //cholmod view of eigen object
    cholmod_sparse *WS_; // WS = (W + S)^{-1}
    cholmod_dense *WSIm_; // WS * (I - m)
    cholmod_factor *L_;
    bool flag_m_, flag_m_gpir_, flag_WS_, flag_WSIm_, flag_W_, flag_W_gpir_;
    double cutoff_; // sparseness cutoff for W.
    cholmod_common Common_, *c_;

};

IMPISD_END_NAMESPACE

#endif /* IMP_ISD_USE_CHOLMOD */

#endif  /* IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_SPARSE_H */
