/**
 *  \file IMP/isd/MultivariateFNormalSufficientSparse.h
 *  \brief Normal distribution of Function
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_SPARSE_H
#define IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_SPARSE_H

#include <IMP/isd/isd_config.h>

#ifdef IMP_ISD_USE_CHOLMOD

#include <IMP/macros.h>
#include <IMP/kernel/Model.h>
#include <IMP/constants.h>
#include <math.h>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unsupported/Eigen/CholmodSupport>
#include <ufsparse/cholmod.h>


IMPISD_BEGIN_NAMESPACE
using Eigen::SparseMatrix;
using Eigen::MatrixXd;
using Eigen::VectorXd;

//! MultivariateFNormalSufficientSparse
/** Probability density function and -log(p) of multivariate normal
    distribution of N M-variate observations.

   \f[ p(x_1,\cdots,x_N|\mu,F,\Sigma)
     = \left((2\pi)^M|\Sigma|\right)^{-N/2} J(F)
    \exp\left(-\frac{1}{2}
     \sum_{i=1}^N {}^t(F(\mu) - F(x_i))\Sigma^{-1}(F(\mu)-F(x_i))
        \right)
    \f]
    which is implemented as
   \f[ p(x_1,\cdots,x_N|\mu,F,\Sigma) = ((2\pi)^M|\Sigma|)^{-N/2} J(F)
    \exp\left(-\frac{N}{2} {}^t\epsilon \Sigma^{-1} \epsilon\right)
    \exp\left(-\frac{1}{2} \operatorname{tr}(W\Sigma^{-1})\right)
    \f]
    where
    \f[\epsilon = (F(\mu)- \overline{F(x)}) \quad
    \overline{F(x)} = \frac{1}{N} \sum_{i=1}^N F(x_i)\f]
    \f( W = \sum_{i=1}^N (F(x_i) - \overline{F(x)}){}^t(F(x_i)
            - \overline{F(x)}) \f)

   Set J(F) to 1 if you want the multivariate normal distribution.
   The distribution is normalized with respect to the matrix variable X.
   The Sufficient statistics are calculated at initialization.

   Example: if F is the log function, the multivariate F-normal distribution
   is the multivariate lognormal distribution with mean \f$\mu\f$ and
   standard deviation \f$\Sigma\f$.

   \note This is an implementation of the matrix normal distribution for F(X),
   where rows of F(X) are independent and homoscedastic (they represent
   repetitions of the same experiment), but columns might be correlated, though
   the provided matrix.

   \note For now, F must be monotonically increasing, so that J(F) > 0. The
   program will not check for that. The inverse of \f$\Sigma\f$ is computed at
   creation time, along with the sufficient statistics and \f$\epsilon\f$.

   \note All observations must be given, so if you have missing data you might
   want to do some imputation on it first.

   \note It uses cholmod and expects a cholmod_common struct as last argument
   of constructor. cholmod_start should be called beforehand, and
   cholmod_finish after destruction of all instances of this class.

   \note References:
   - Multivariate Likelihood:
     - Box and Tiao, "Bayesian Inference in Statistical Analysis",
       Addison-Wesley publishing company, 1973, pp 423.
   - Factorization in terms of sufficient statistics:
     - Daniel Fink, "A Compendium of Conjugate Priors", online, May 1997,
       p.40-41.
   - Matrix calculations for derivatives:
     - Petersen and Pedersen, "The Matrix Cookbook", 2008, matrixcookbook.com
   - Useful reading on missing data (for the case of varying Nobs):
     - Little and Rubin, "Statistical Analysis with Missing Data",
       2nd ed, Wiley, 2002, Chapters 6,7 and 11.
 */
class IMPISDEXPORT MultivariateFNormalSufficientSparse : public Object
{
public:
  //! Initialize with all observed data
  /** \param(in) FX matrix of observations with M columns and N rows.
      \param(in) JF determinant of Jacobian of F with respect to
                    observation matrix X.
      \param(in) FM mean vector \f$F(\mu)\f$ of size M.
      \param(in) Sigma MxM variance-covariance matrix \f$\Sigma\f$ in sparse
                 format. Will use symmetry by only considering the upper corner.
      \param(in) c the control struct for cholmod.
      \param(in) cutoff when to consider that coefficients of the W matrix are
                 zero.
   */
  MultivariateFNormalSufficientSparse(const MatrixXd& FX, double JF,
            const VectorXd& FM, const SparseMatrix<double>& Sigma,
            cholmod_common *c, double cutoff=1e-7);

  //! Initialize with sufficient statistics
  /** \param(in) Fbar M-dimensional vector of mean observations.
      \param(in) JF determinant of Jacobian of F with respect to
                    observation matrix X.
      \param(in) FM M-dimensional true mean vector \f$\mu\f$.
      \param(in) Nobs number of observations for each variable.
      \param(in) W MxM matrix of sample variance-covariances, sparse.
      \param(in) Sigma MxM variance-covariance matrix Sigma, sparse.
      \param(in) c the control struct for cholmod.
   */
  MultivariateFNormalSufficientSparse(const VectorXd& Fbar, double JF,
            const VectorXd& FM, int Nobs, const SparseMatrix<double>& W,
            const SparseMatrix<double>& Sigma, cholmod_common *c);

  //! probability density function
  double density() const;

  //! energy (score) functions, aka -log(p)
  double evaluate() const;

  //! gradient of the energy wrt the mean F(M)
  cholmod_dense *evaluate_derivative_FM() const;

  //! gradient of the energy wrt the variance-covariance matrix Sigma
  cholmod_sparse *evaluate_derivative_Sigma() const;

  //! change of parameters
  void set_FX(const MatrixXd& f, double cutoff=1e-7);

  void set_JF(double f);

  void set_FM(const VectorXd& f);

  void set_Fbar(const VectorXd& f);

  void set_W(const SparseMatrix<double>& f);

  void set_Sigma(const SparseMatrix<double>& f);

  /* remaining stuff */
  IMP_OBJECT_METHODS(MultivariateFNormalSufficientSparse);
  /*IMP_OBJECT_INLINE(MultivariateFNormalSufficientSparse,
          out << "MultivariateFNormalSufficientSparse: "
              << N_ << " observations of "
              <<  M_ << " variables " <<std::endl,
            {
                cholmod_free_sparse(&W_,c_);
                cholmod_free_sparse(&P_,c_);
                cholmod_free_sparse(&Sigma_,c_);
                cholmod_free_dense(&epsilon_,c_);
                cholmod_free_factor(&L_,c_);
                cholmod_finish(c_);
                });*/

 private:

  /* return trace(W.P), O(M^2) */
  double trace_WP() const;

  /* return transpose(epsilon)*P*epsilon, O(M^2) */
  double mean_dist() const;

  /* return P*epsilon*transpose(P*epsilon), O(M^2) */
  cholmod_sparse *compute_PTP() const;

  /* return P * W * P, O(M^3) */
  cholmod_sparse *compute_PWP() const;

  /* compute epsilon, W and Fbar, O(N*M^2) */
  void compute_sufficient_statistics(double cutoff);

  /*computes the discrepancy vector*/
  void compute_epsilon();

  VectorXd FM_, Fbar_;
  double JF_,lJF_,norm_,lnorm_;
  cholmod_sparse *W_, *Sigma_, *P_, *PW_; // matrices
  cholmod_dense *epsilon_; // vectors
  cholmod_common *c_;
  cholmod_factor *L_;
  MatrixXd FX_;

  int N_; //number of repetitions
  int M_; //number of variables
};

IMPISD_END_NAMESPACE

#endif /* IMP_ISD_USE_CHOLMOD */

#endif  /* IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_SPARSE_H */
