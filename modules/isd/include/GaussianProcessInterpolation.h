/**
 *  \file GaussianProcessInterpolation.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_H
#define IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_H

#include "isd_config.h"
#include <IMP/macros.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/algebra/internal/jama_cholesky.h>
#include <boost/scoped_ptr.hpp>
#include <IMP/isd/functions.h>


IMPISD_BEGIN_NAMESPACE
#ifndef SWIG
using IMP::algebra::internal::TNT::Array1D;
using IMP::algebra::internal::TNT::Array2D;
#endif

class GaussianProcessInterpolationRestraint;

//! GaussianProcessInterpolation
/** This class provides methods to perform bayesian interpolation/smoothing of
 * data using a gaussian process prior on the function to be interpolated.
 * It takes a dataset as input (via its sufficient statistics) along with prior
 * mean and covariance functions. It outputs the value of the posterior mean and
 * covariance functions at points requested by the user.
 */
class IMPISDEXPORT GaussianProcessInterpolation : public Object
{
 public:
     /** Constructor for the gaussian process
      * \param[in] x : a list of coordinates in N-dimensional space
      *                corresponding to the abscissa of each observation
      * \param[in] sample_mean \f$I\f$ : vector of mean observations at each of
      *                                  the previously defined coordinates
      * \param[in] sample_std \f$s\f$ : vector of sample standard deviations
      * \param[in] mean_function \f$m\f$ : a pointer to the prior mean function
      *                                    to use.  Should be compatible with
      *                                    the size of x[i].  
      * \param[in] covariance_function \f$w\f$: prior covariance function.
      *
      * Computes the necessary matrices and inverses when called.
      */
  GaussianProcessInterpolation(FloatsList x,
                               std::vector<double> sample_mean,
                               std::vector<double> sample_std,
                               std::vector<int> n_obs,
                               UnivariateFunction *mean_function,
                               BivariateFunction *covariance_function);

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
  double get_posterior_mean(std::vector<double> x);
  double get_posterior_covariance(std::vector<double> x1, 
                                  std::vector<double> x2);

  // call these if you called update() on the mean or covariance function. 
  // it will force update any internal variables dependent on these functions.
  void force_mean_update();
  void force_covariance_update();

  friend class GaussianProcessInterpolationRestraint;

  IMP_OBJECT(GaussianProcessInterpolation);

 protected:
  //returns updated data vector
  Array1D<double> get_I() const {return I_;}
  //returns updated prior mean vector
  Array1D<double> get_m();
  // returns updated prior covariance vector
  Array1D<double> get_wx_vector(std::vector<double> xval);
  //returns updated data covariance matrix
  Array2D<double> get_S() const {return S_;}
  //returns updated prior covariance matrix
  Array2D<double> get_W();
  //returns updated (W+S)^{-1}
  Array1D<double> get_WS();
  //returns updated (W+S)^{-1}(I-m)
  Array1D<double> get_WSIm();

 private:

  // ensures the mean/covariance function has updated parameters. Signals an
  // update by changing the state flags.
  void update_flags_mean();
  void update_flags_covariance();

  // compute prior covariance matrix
  void compute_W();
  // compute \f$(\mathbf{W} + \mathbf{S})^{-1}\f$ (if necessary).
  void compute_WS();
  // compute (W+S)^{-1} (I-m)
  void compute_WSIm();

  // compute mean observations
  void compute_I(std::vector<double> mean);
  // compute diagonal covariance matrix of observations
  void compute_S(std::vector<double> std, std::vector<int> n);
  // compute prior mean vector
  void compute_m();

 private:
    unsigned N_; // number of dimensions of the abscissa
    unsigned M_; // number of observations to learn from
    FloatsList x_; // abscissa
    std::vector<int> n_obs_; // number of observations
    // pointer to the prior mean function
    IMP::internal::OwnerPointer<UnivariateFunction> mean_function_; 
    // pointer to the prior covariance function
    IMP::internal::OwnerPointer<BivariateFunction> covariance_function_;
    Array1D<double> I_,m_,wx_; 
    Array2D<double> S_,W_,WS_; // WS = (W + S)^{-1}
    Array1D<double> WSIm_; // WS * (I - m)
    bool flag_m_, flag_WS_, flag_WSIm_, flag_W_;
    boost::scoped_ptr<algebra::internal::JAMA::Cholesky<double> > Cholesky_;

};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_GAUSSIAN_PROCESS_INTERPOLATION_H */
