/**
 *  \file MultivariateFNormalSufficientDiagonalW.h
 *  \brief Normal distribution of Function
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_DIAGONAL_W_H
#define IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_DIAGONAL_W_H

#include "isd_config.h"
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

//! MultivariateFNormalSufficientDiagonalW
/** Probability density function and -log(p) of multivariate normal
 * distribution of N M-variate observations.
 *
 * \f[ p(x_1,\cdots,x_N|\mu,F,\Sigma) = \left((2\pi)^M|\Sigma|\right)^{-N/2}
 *  J(F)
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
 *  \f( W = \sum_{i=1}^N (F(x_i) - \overline{F(x)}){}^t(F(x_i)
 *          - \overline{F(x)}) \f)
 *
 * Set J(F) to 1 if you want the multivariate normal distribution.
 * The distribution is normalized with respect to the matrix variable X.
 * The SufficientDiagonalW statistics are calculated at initialization.
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

class IMPISDEXPORT MultivariateFNormalSufficientDiagonalW : public Object
{

private:

  VectorXd FM_, Fbar_, epsilon_,Peps_;
  double JF_,lJF_,norm_,lnorm_;
  MatrixXd P_,W_,Sigma_,FX_,PW_,precond_;
  int N_; //number of repetitions
  int M_; //number of variables
  Eigen::LLT<MatrixXd, Eigen::Upper> ldlt_;
  //flags are true if the corresponding object is up to date.
  bool flag_FM_, flag_FX_, flag_Fbar_,
       flag_W_, flag_Sigma_, flag_epsilon_,
       flag_PW_, flag_P_, flag_ldlt_, flag_norms_,
       flag_Peps_;
  //cg-related variables
  bool use_cg_, first_PW_, first_PWP_;
  double cg_tol_;
  IMP::Pointer<internal::ConjugateGradientEigen> cg_;

  internal::CallTimer<IMP_MVN_TIMER_NFUNCS> timer_;

 public:
     /** Initialize with all observed data
 * \param(in) F(X) matrix of observations with M columns and N rows.
 * \param(in) J(F) determinant of Jacobian of F with respect to
 *                 observation matrix X.
 * \param(in) F(M) mean vector \f$F(\mu)\f$ of size M.
 * \param(in) Sigma : MxM variance-covariance matrix \f$\Sigma\f$.
 * */
  MultivariateFNormalSufficientDiagonalW(const MatrixXd& FX, double JF,
            const VectorXd& FM, const MatrixXd& Sigma);

     /** Initialize with sufficient statistics
 * \param(in) Fbar : M-dimensional vector of mean observations.
 * \param(in) J(F) determinant of Jacobian of F with respect to observation
 *                  matrix X.
 * \param(in) F(M) : M-dimensional true mean vector \f$\mu\f$.
 * \param(in) Nobs : number of observations for each variable.
 * \param(in) W : MxM matrix of sample variance-covariances.
 * \param(in) Sigma : MxM variance-covariance matrix Sigma.
 * */
  MultivariateFNormalSufficientDiagonalW(const VectorXd& Fbar, double JF,
            const VectorXd& FM, int Nobs,  const MatrixXd& W,
            const MatrixXd& Sigma);

  /* probability density function */
  double density() const;

  /* energy (score) functions, aka -log(p) */
  double evaluate() const;

  /* gradient of the energy wrt the mean F(M) */
  VectorXd evaluate_derivative_FM() const;

  /* gradient of the energy wrt the variance-covariance matrix Sigma */
  MatrixXd evaluate_derivative_Sigma() const;

  /* change of parameters */
  void set_FX(const MatrixXd& f);
  MatrixXd get_FX() const;

  void set_FM(const VectorXd& f);
  VectorXd get_FM() const;

  void set_Fbar(const VectorXd& f);
  VectorXd get_Fbar() const;

  void set_W(const MatrixXd& f);
  MatrixXd get_W() const;

  void set_Sigma(const MatrixXd& f);
  MatrixXd get_Sigma() const;

  //if you want to force a recomputation of all stored variables
  void reset_flags();

  // use conjugate gradients (default false)
  void set_use_cg(bool use, double tol);

  // print runtime statistics
  void stats() const;

  /* remaining stuff */
  IMP_OBJECT_INLINE(MultivariateFNormalSufficientDiagonalW,
          out << "MultivariateFNormalSufficientDiagonalW: "
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
  Eigen::LLT<MatrixXd, Eigen::Upper> get_ldlt() const;
  void set_ldlt(const Eigen::LLT<MatrixXd, Eigen::Upper>& ldlt);

  // compute determinant and norm
  void set_norms(double norm, double lnorm);
  std::vector<double> get_norms() const;

  /* return trace(W.P) */
  double trace_WP() const;

  /* return transpose(epsilon)*P*epsilon */
  double mean_dist() const;

  /* return P*epsilon*transpose(P*epsilon) */
  MatrixXd compute_PTP() const;

  /* return P * W * P, O(M^2) */
  MatrixXd compute_PWP() const;

  /*computes the discrepancy vector*/
  void compute_epsilon();


};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_DIAGONAL_W_H */
