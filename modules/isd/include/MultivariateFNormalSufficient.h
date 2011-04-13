/**
 *  \file MultivariateFNormalSufficient.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_H
#define IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_H

#include "isd_config.h"
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/algebra/internal/jama_lu.h>
#include <boost/scoped_ptr.hpp>

IMPISD_BEGIN_NAMESPACE

//! MultivariateFNormalSufficient
/** Probability density function and -log(p) of multivariate normal
 * distribution of N M-variate observations.
 * \param[in] F(A) matrix of observations with M columns and N rows.
 * \param[in] J(A) determinant of Jacobian of F with respect to vector A. Set to
 * 1 if you want the multivariate normal distribution.
 * \param[in] F(M) mean vector of size M.
 * \param[in] MxM precision matrix P (inverse of the variance-covariance matrix Sigma).
 * The distribution is normalized with respect to the vector variable A.
 * The Sufficient statistics are calculated at initialization.
 *
 *  Example: if F is the log function, the multivariate F-normal distribution is the
 *  multivariate lognormal distribution with mean M and standard deviation sigma.
 *
 *  \note For now, F must be monotonically increasing, so that JA > 0. The
 *  program will not check for that. Working with Sigma is very slow so the
 *  distribution asks for the precision matrix instead, e.g. the inverse of
 *  Sigma.
 *
 *  \note Based on Box and Tiao, "Bayesian Inference in Statistical Analysis",
 *  Addison-Wesley publishing company, 1973, pp 423.
 *
 */

class IMPISDEXPORT MultivariateFNormalSufficient : public Object
{
 public:
  MultivariateFNormalSufficient(algebra::internal::TNT::Array2D<double> FA, 
          double JA, algebra::internal::TNT::Array1D<double> FM, 
          algebra::internal::TNT::Array2D<double> P);

  /* probability density function */
  double density() const;
    
  /* energy (score) functions, aka -log(p) */
  double evaluate() const;

  /* gradient of the energy wrt the mean F(M) */
  algebra::internal::TNT::Array1D<double> evaluate_derivative_FM() const;

  /* gradient of the energy wrt the precision matrix P */
  algebra::internal::TNT::Array2D<double> evaluate_derivative_P() const;
  
  /* change of parameters */
  void set_FA(algebra::internal::TNT::Array2D<double> f);

  void set_JA(double f);

  void set_FM(algebra::internal::TNT::Array1D<double> f);

  void set_P(algebra::internal::TNT::Array2D<double> f);

  /* remaining stuff */
  IMP_OBJECT_INLINE(MultivariateFNormalSufficient, out << "MultivariateFNormalSufficient: " << N_ << " observations of " <<  M_ << " variables " <<std::endl, {});

 private:

  /* compare two matrices */
  bool are_equal(algebra::internal::TNT::Array1D<double> A,
                 algebra::internal::TNT::Array1D<double> B) const;

  bool are_equal(algebra::internal::TNT::Array2D<double> A,
                 algebra::internal::TNT::Array2D<double> B) const;

  /* return trace(P.S) */
  double trace_PS() const;

  /* compute epsilon = FA-FM and S=trans(epsilon).epsilon */
  void compute_sufficient_statistics_matrix();

  algebra::internal::TNT::Array1D<double> FM_;
  double JA_,lJA_,norm_,lnorm_;
  algebra::internal::TNT::Array2D<double> P_,FA_,S_,epsilon_,Sigma_;
  boost::scoped_ptr<algebra::internal::JAMA::LU<double> > LUP_;
  int N_; //number of repetitions
  int M_; //number of variables
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_MULTIVARIATE_FNORMAL_SUFFICIENT_H */
