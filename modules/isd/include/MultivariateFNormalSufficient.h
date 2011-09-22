/**
 *  \file MultivariateFNormalSufficient.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_H
#define IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_H

#include "isd_config.h"
#include <IMP/macros.h>
#include <IMP/Model.h>
#include <IMP/constants.h>
#include <math.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/algebra/internal/jama_cholesky.h>
#include <boost/scoped_ptr.hpp>


IMPISD_BEGIN_NAMESPACE
using IMP::algebra::internal::TNT::Array1D;
using IMP::algebra::internal::TNT::Array2D;

//! MultivariateFNormalSufficient
/** Probability density function and -log(p) of multivariate normal
 * distribution of N M-variate observations.
 *
 * \f[ p(x_1,\cdots,x_N|\mu,F,\Sigma) = \left((2\pi)^M|\Sigma|\right)^{-N/2} J(F) 
 *  \exp\left(-\frac{1}{2}
 *   \sum_{i=1}^N {}^t(F(\mu) - F(x_i))\Sigma^{-1}(F(\mu)-F(x_i))
 *      \right) 
 *  \f]
 *  which is implemented as
 * \f[ p(x_1,\cdots,x_N|\mu,F,\Sigma) = ((2\pi)^M|\Sigma|)^{-N/2} J(F) 
 *  \exp\left(-\frac{N}{2} {}^t\epsilon \Sigma^{-1} \epsilon\right)
 *  \eps\left(-\frac{1}{2} \tr(W\Sigma^{-1})\right)
 *  \f]
 *  where
 *  \f[\epsilon = (F(\mu)- \overline{F(x)}) \quad 
 *  \overline{F(x)} = \frac{1}{N} \sum_{i=1}^N F(x_i)\f]
 *  \f[ W = \sum_{i=1}^N (F(x_i) - \overline{F(x)}){}^t(F(x_i) - \overline{F(x)}) \f]
 *
 * Set J(F) to 1 if you want the multivariate normal distribution.
 * The distribution is normalized with respect to the matrix variable X.
 * The Sufficient statistics are calculated at initialization.
 *
 *  Example: if F is the log function, the multivariate F-normal distribution is the
 *  multivariate lognormal distribution with mean \f$\mu\f$ and standard deviation \f$\Sigma\f$.
 *
 *  \note This is an implementation of the matrix normal distribution for F(X),
 *  where rows of F(X) are independent and homoscedastic (they represent
 *  repetitions of the same experiment), but columns might be correlated, though
 *  the provided matrix.
 *
 *  \note For now, F must be monotonically increasing, so that J(F) > 0. The
 *  program will not check for that. The inverse of \f$\Sigma\f$ is computed at
 *  creation time, along with the sufficient statistics and \f$\epsilon\f$.
 *
 *  \note All observations must be given, so if you have missing data you might
 *  want to do some imputation on it first.
 *
 *  \note References:
 *  - Multivariate Likelihood:
 *  Box and Tiao, "Bayesian Inference in Statistical Analysis",
 *  Addison-Wesley publishing company, 1973, pp 423.
 *  - Factorization in terms of sufficient statistics:
 *  Daniel Fink, "A Compendium of Conjugate Priors", online, May 1997, p.40-41.
 *  - Matrix calculations for derivatives:
 *  Petersen and Pedersen, "The Matrix Cookbook", 2008, matrixcookbook.com
 *  - Useful reading on missing data (for the case of varying Nobs):
 *  Little and Rubin, "Statistical Analysis with Missing Data", 2nd ed, Wiley,
 *  2002, Chapters 6,7 and 11.
 *
 */

class IMPISDEXPORT MultivariateFNormalSufficient : public Object
{
 public:
     /** Initialize with all observed data
 * \param[in] F(X) matrix of observations with M columns and N rows.
 * \param[in] J(F) determinant of Jacobian of F with respect to observation matrix X. 
 * \param[in] F(M) mean vector \f$F(\mu)\f$ of size M.
 * \param[in] Sigma : MxM variance-covariance matrix \f$\Sigma\f$.
 * */
  MultivariateFNormalSufficient(Array2D<double> FX, double JF, 
            Array1D<double> FM, Array2D<double> Sigma);

     /** Initialize with sufficient statistics
 * \param[in] Fbar : M-dimensional vector of mean observations.
 * \param[in] J(F) determinant of Jacobian of F with respect to observation matrix X. 
 * \param[in] F(M) : M-dimensional true mean vector \f$\mu\f$.
 * \param[in] Nobs : number of observations for each variable.
 * \param[in] W : MxM matrix of sample variance-covariances.
 * \param[in] Sigma : MxM variance-covariance matrix Sigma.
 * */
  MultivariateFNormalSufficient(Array1D<double> Fbar, double JF, 
            Array1D<double> FM, int Nobs,  Array2D<double> W, 
            Array2D<double> Sigma);

  /* probability density function */
  double density() const;
    
  /* energy (score) functions, aka -log(p) */
  double evaluate() const;

  /* gradient of the energy wrt the mean F(M) */
  Array1D<double> evaluate_derivative_FM() const;

  /* gradient of the energy wrt the variance-covariance matrix Sigma */
  Array2D<double> evaluate_derivative_Sigma() const;
  
  /* change of parameters */
  void set_FX(Array2D<double> f);

  void set_JF(double f);

  void set_FM(Array1D<double> f);

  void set_Fbar(Array1D<double> f);

  void set_Nobs(int f){Nobs_=f;}

  void set_W(Array2D<double> f);

  void set_Sigma(Array2D<double> f);

  /* speed up calculations with W by considering values smaller than val 
   * to be zero.
   */
  void set_W_nonzero(bool yes, double val=1e-7);

  /* remaining stuff */
  IMP_OBJECT_INLINE(MultivariateFNormalSufficient, out << "MultivariateFNormalSufficient: " << N_ << " observations of " <<  M_ << " variables " <<std::endl, {});

 private:

  /* compare two matrices */
  bool are_equal(Array1D<double> A,
                 Array1D<double> B) const;

  bool are_equal(Array2D<double> A,
                 Array2D<double> B) const;

  /* return trace(W.P), O(M^2) */
  double trace_WP() const;

  /* return transpose(epsilon)*P*epsilon, O(M^2) */
  double mean_dist() const;

  /* return P*epsilon*transpose(P*epsilon), O(M^2) */
  Array2D<double> compute_PTP() const;

  /* return P * W * P, O(M^3) */
  Array2D<double> compute_PWP() const;

  /* compute epsilon, W and Fbar, O(N*M^2) */
  void compute_sufficient_statistics();

  /*computes the discrepancy vector*/
  void compute_epsilon();

  Array1D<double> FM_, Fbar_, epsilon_;
  Array1D<int> Nobs_;
  double JF_,lJF_,norm_,lnorm_;
  Array2D<double> P_,W_,Sigma_,FX_,WP_ ;
  boost::scoped_ptr<algebra::internal::JAMA::Cholesky<double> > CholeskySigma_;
  int N_; //number of repetitions
  int M_; //number of variables
  bool W_is_diagonal_;
  bool W_is_zero_;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_H */
