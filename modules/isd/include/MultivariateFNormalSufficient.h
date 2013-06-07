/**
 *  \file IMP/isd/MultivariateFNormalSufficient.h
 *  \brief Normal distribution of Function
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_H
#define IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_H

#include <IMP/isd/isd_config.h>
#include "internal/timer.h"
#include <IMP/macros.h>
#include <IMP/Model.h>
#include <IMP/constants.h>
#include <IMP/base/Object.h>
#include <math.h>
#include <Eigen/Dense>
#include <Eigen/Cholesky>
#include <IMP/isd/internal/cg_eigen.h>

IMPISD_BEGIN_NAMESPACE
using Eigen::MatrixXd;
using Eigen::VectorXd;

//IMP_MVN_TIMER_NFUNCS is the number of functions used by the timer
#define IMP_MVN_TIMER_NFUNCS 11

//! MultivariateFNormalSufficient
/** Probability density function and -log(p) of multivariate normal
 * distribution of N M-variate observations.
 *
 * \f[ p(x_1,\cdots,x_N|\mu,F,\Sigma) =
 *  \left((2\pi\sigma^2)^M|\Sigma|\right)^{-N/2} J(F)
 *  \exp\left(-\frac{1}{2\sigma^2}
 *   \sum_{i=1}^N {}^t(F(\mu) - F(x_i))\Sigma^{-1}(F(\mu)-F(x_i))
 *      \right)
 *  \f]
 *  which is implemented as
 * \f[ p(x_1,\cdots,x_N|\mu,F,\Sigma) = ((2\pi\sigma^2)^M|\Sigma|)^{-N/2} J(F)
 *  \exp\left(-\frac{N}{2\sigma^2} {}^t\epsilon \Sigma^{-1} \epsilon\right)
 *  \exp\left(-\frac{1}{2\sigma^2} \operatorname{tr}(W\Sigma^{-1})\right)
 *  \f]
 *  where
 *  \f[\epsilon = (F(\mu)- \overline{F(x)}) \quad
 *  \overline{F(x)} = \frac{1}{N} \sum_{i=1}^N F(x_i)\f]
 *  and
 *  \f[W=\sum_{i=1}^N(F(x_i)-\overline{F(x)}){}^t(F(x_i)-\overline{F(x)}) \f]
 *
 * \f$\sigma\f$ is a multiplicative scaling factor that factors out of the
 * \f$\Sigma\f$ covariance matrix. It is set to 1 by default and its intent is
 * to avoid inverting the \f$\Sigma\f$ matrix unless necessary.
 *
 * Set J(F) to 1 if you want the multivariate normal distribution.
 * The distribution is normalized with respect to the matrix variable X.
 * The Sufficient statistics are calculated at initialization.
 *
 *  Example: if F is the log function, the multivariate F-normal distribution
 *  is the multivariate lognormal distribution with mean \f$\mu\f$ and
 *  standard deviation \f$\Sigma\f$.
 *
 *  \note This is an implementation of the matrix normal distribution for F(X),
 *  where rows of F(X) are independent and homoscedastic (they represent
 *  repetitions of the same experiment), but columns might be correlated,
 *  though the provided matrix.
 *
 *  \note For now, F must be monotonically increasing, so that J(F) > 0. The
 *  program will not check for that. Uses a Cholesky (\f$LDL^T\f$)
 *  decomposition of \f$\Sigma\f$, which is recomputed when needed.
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

class IMPISDEXPORT MultivariateFNormalSufficient : public base::Object
{

private:

  VectorXd FM_, Fbar_, epsilon_,Peps_;
  double JF_,lJF_,norm_,lnorm_;
  MatrixXd P_,W_,Sigma_,FX_,PW_;
  int N_; //number of repetitions
  int M_; //number of variables
  //Eigen::LLT<MatrixXd, Eigen::Upper> ldlt_;
  Eigen::LDLT<MatrixXd, Eigen::Upper> ldlt_;
  //flags are true if the corresponding object is up to date.
  bool flag_FM_, flag_FX_, flag_Fbar_,
       flag_W_, flag_Sigma_, flag_epsilon_,
       flag_PW_, flag_P_, flag_ldlt_, flag_norms_,
       flag_Peps_;
  //cg-related variables
  MatrixXd precond_;
  bool use_cg_, first_PW_, first_PWP_;
  double cg_tol_;
  double factor_;
  base::Pointer<internal::ConjugateGradientEigen> cg_;

  internal::CallTimer<IMP_MVN_TIMER_NFUNCS> timer_;

 public:
     /** Initialize with all observed data
 * \param [in] FX F(X) matrix of observations with M columns and N rows.
 * \param [in] JF J(F) determinant of Jacobian of F with respect to
 *                 observation matrix X.
 * \param [in] FM F(M) mean vector \f$F(\mu)\f$ of size M.
 * \param [in] Sigma : MxM variance-covariance matrix \f$\Sigma\f$.
 * \param [in] factor : multiplicative factor (default 1)
 * */
  MultivariateFNormalSufficient(const MatrixXd& FX, double JF,
            const VectorXd& FM, const MatrixXd& Sigma, double factor=1);

     /** Initialize with sufficient statistics
 * \param [in] Fbar : M-dimensional vector of mean observations.
 * \param [in] JF J(F) determinant of Jacobian of F with respect to observation
 *                  matrix X.
 * \param [in] FM F(M) : M-dimensional true mean vector \f$\mu\f$.
 * \param [in] Nobs : number of observations for each variable.
 * \param [in] W : MxM matrix of sample variance-covariances.
 * \param [in] Sigma : MxM variance-covariance matrix Sigma.
 * \param [in] factor : multiplicative factor (default 1)
 * */
  MultivariateFNormalSufficient(const VectorXd& Fbar, double JF,
            const VectorXd& FM, int Nobs,  const MatrixXd& W,
            const MatrixXd& Sigma, double factor=1);

  /* probability density function */
  double density() const;

  /* energy (score) functions, aka -log(p) */
  double evaluate() const;

  /* gradient of the energy wrt the mean F(M) */
  VectorXd evaluate_derivative_FM() const;

  /* gradient of the energy wrt the variance-covariance matrix Sigma */
  MatrixXd evaluate_derivative_Sigma() const;

  // derivative wrt scalar factor
  double evaluate_derivative_factor() const;

  /* second derivative wrt FM and FM */
  MatrixXd evaluate_second_derivative_FM_FM() const;

  /* second derivative wrt FM(l) and Sigma
   * row and column indices in the matrix returned are for Sigma
   */
  MatrixXd evaluate_second_derivative_FM_Sigma(unsigned l) const;

  /* second derivative wrt Sigma and Sigma(k,l) */
  MatrixXd evaluate_second_derivative_Sigma_Sigma(unsigned k, unsigned l) const;


  /* change of parameters */
  void set_FX(const MatrixXd& f);
  MatrixXd get_FX() const;

  void set_Fbar(const VectorXd& f);
  VectorXd get_Fbar() const;

  void set_jacobian(double f);
  double get_jacobian() const;
  void set_minus_log_jacobian(double f); //-log(J)
  double get_minus_log_jacobian() const;

  void set_FM(const VectorXd& f);
  VectorXd get_FM() const;

  void set_W(const MatrixXd& f);
  MatrixXd get_W() const;

  void set_Sigma(const MatrixXd& f);
  MatrixXd get_Sigma() const;

  void set_factor(double f);
  double get_factor() const;

  //if you want to force a recomputation of all stored variables
  void reset_flags();

  // use conjugate gradients (default false)
  void set_use_cg(bool use, double tol);

  // print runtime statistics
  void stats() const;

  //return Sigma's eigenvalues from smallest to biggest
  VectorXd get_Sigma_eigenvalues() const;

  //return Sigma's condition number
  double get_Sigma_condition_number() const;

  //Solve for Sigma*X = B, yielding X
  MatrixXd solve(MatrixXd B) const;

  /* return transpose(epsilon)*P*epsilon */
  double get_mean_square_residuals() const;

  /* return minus exponent
   *  \f[-\frac{1}{2\sigma^2}
   *   \sum_{i=1}^N {}^t(F(\mu) - F(x_i))\Sigma^{-1}(F(\mu)-F(x_i)) \f]
   */
  double get_minus_exponent() const;

  /* return minus log normalization
   * \f[\frac{N}{2}\left(\log(2\pi\sigma^2) + \log |\Sigma|\right)
   * -\log J(F) \f]
   */
  double get_minus_log_normalization() const;

  /* remaining stuff */
  IMP_OBJECT_INLINE(MultivariateFNormalSufficient,
          out << "MultivariateFNormalSufficient: "
          << N_ << " observations of "
          <<  M_ << " variables " <<std::endl,
          {});

 private:

  //conjugate gradient init
  void setup_cg();

  //precision matrix
  MatrixXd get_P() const;
  void set_P(const MatrixXd& P);

  //precision * W
  MatrixXd get_PW() const;
  MatrixXd compute_PW_direct() const;
  MatrixXd compute_PW_cg() const;
  void set_PW(const MatrixXd& PW);

  //precision * epsilon
  VectorXd get_Peps() const;
  void set_Peps(const VectorXd& Peps);

  // epsilon = Fbar - FM
  VectorXd get_epsilon() const;
  void set_epsilon(const VectorXd& eps);

  // gets factorization object
  //Eigen::LLT<MatrixXd, Eigen::Upper> get_ldlt() const;
  Eigen::LDLT<MatrixXd, Eigen::Upper> get_ldlt() const;
  void set_ldlt(const Eigen::LDLT<MatrixXd, Eigen::Upper>& ldlt);

  // compute determinant and norm
  void set_norms(double norm, double lnorm);
  std::vector<double> get_norms() const;

  /* return trace(W.P) */
  double trace_WP() const;

  /* return P*epsilon*transpose(P*epsilon) */
  MatrixXd compute_PTP() const;

  /* return P * W * P, O(M^2) */
  MatrixXd compute_PWP() const;

  /*computes the discrepancy vector*/
  void compute_epsilon();


};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_H */
