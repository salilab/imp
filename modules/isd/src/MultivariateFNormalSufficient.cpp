/**
 *  \file MultivariateFNormalSufficient.cpp  
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/MultivariateFNormalSufficient.h>
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

  MultivariateFNormalSufficient::MultivariateFNormalSufficient(
          algebra::internal::TNT::Array2D<double> FA, double JA, 
          algebra::internal::TNT::Array1D<double> FM,
          algebra::internal::TNT::Array2D<double> P) 
    {
        M_=P.dim1();
        N_=FA.dim1();
        if (N_ <= 0){
            IMP_THROW("please provide at least one observation per dimension", ModelException);
        }
        if (M_ <= 0){
            IMP_THROW("please provide a nonempty mean vector", ModelException);
        }
        FM_=FM.copy();
        FA_=FA.copy();
        set_JA(JA);
        compute_sufficient_statistics_matrix();
        set_P(P);
    }

  /* probability density function */
  double MultivariateFNormalSufficient::density() const
  { 
      return norm_*JA_*exp(-0.5*trace_PS());
  }
 
  /* energy (score) functions, aka -log(p) */
  double MultivariateFNormalSufficient::evaluate() const 
  { 
      return lnorm_ + lJA_ + 0.5*trace_PS();
  }

  algebra::internal::TNT::Array1D<double> 
   MultivariateFNormalSufficient::evaluate_derivative_FM() const
  { 
      // d(-log(p))/d(FM) = - P * trans(epsilon) * J_N  (J_N is a vector of ones).
      algebra::internal::TNT::Array1D<double> intval(M_);
      algebra::internal::TNT::Array1D<double> retval(M_);
      for (int i=0; i<M_; i++) {
          intval[i]=0.0;
          for (int j=0; j<N_; j++) {
              intval[i] += epsilon_[j][i];
          }
      }
      for (int i=0; i<M_; i++) {
          retval[i]=0.0;
          for (int j=0; j<M_; j++) {
              retval[i] += - P_[i][j]*intval[j];
          }
      }
      return retval;
  }

  algebra::internal::TNT::Array2D<double> 
   MultivariateFNormalSufficient::evaluate_derivative_P() const
  { 
      //d(-log(p))/dP = 1/2 * ( S - N*Sigma )
      algebra::internal::TNT::Array2D<double> R(M_, M_);
      for (int i=0; i<M_; i++){
          for (int j=0; j<M_; j++){
              R[i][j] = 0.5*(S_[i][j]-N_*Sigma_[i][j]);
          }
      }
      return R;
  }
  
  bool MultivariateFNormalSufficient::are_equal(algebra::internal::TNT::Array1D<double> A,
                 algebra::internal::TNT::Array1D<double> B) const {
      if (A.dim1() != B.dim1()) return false;
      for (int i=0; i<A.dim1(); i++){
              if (A[i] != B[i]) return false;
      }
      return true;
  }

  bool MultivariateFNormalSufficient::are_equal(algebra::internal::TNT::Array2D<double> A,
                 algebra::internal::TNT::Array2D<double> B) const {
      if (A.dim1() != B.dim1()) return false;
      if (A.dim2() != B.dim2()) return false;
      for (int i=0; i<A.dim1(); i++){
          for (int j=0; j<A.dim2(); j++){
              if (A[i][j] != B[i][j]) return false;
          }
      }
      return true;
  }

  void MultivariateFNormalSufficient::set_FA(algebra::internal::TNT::Array2D<double> FA) {
    if (!are_equal(FA,FA_)){
        if (FA_.dim1() != N_) {
            IMP_THROW("size mismatch for FA in the number of repetitions", ModelException);
            }
        if (FA_.dim2() != M_) {
            IMP_THROW("size mismatch for FA and P", ModelException);
            }
        FA_=FA.copy();
        compute_sufficient_statistics_matrix();
    }
  }

  void MultivariateFNormalSufficient::set_JA(double f) {
    JA_=f;
    lJA_=-log(JA_);
  }

  void MultivariateFNormalSufficient::set_FM(algebra::internal::TNT::Array1D<double> FM) {
    if (!are_equal(FM,FM_)){
        if (FM_.dim1() != M_) {
            IMP_THROW("size mismatch for FM and P", ModelException);
            }
        FM_=FM.copy();
        compute_sufficient_statistics_matrix();
    }
  }

  void MultivariateFNormalSufficient::set_P(algebra::internal::TNT::Array2D<double> P)  
  {
    if (!are_equal(P,P_)) {
        if (P.dim2() != P.dim1()) {
            IMP_THROW("need a square matrix!", ModelException);
            }
        P_=P;
        // compute LU decomposition for determinant and inverse
        LUP_.reset(new algebra::internal::JAMA::LU<double> (P_));
        // determinant and derived constants
        double detP=LUP_->det();
        norm_=pow(2*IMP::PI, -double(N_*M_)/2.0) * pow(detP, double(N_)/2.0);
        lnorm_=double(N_*M_)/2 * log(2*IMP::PI) - double(N_)/2 * log(detP);
        //inverse (taken from TNT website)
        algebra::internal::TNT::Array2D<double> id(M_, M_, 0.0);
        for (int i=0; i<M_; i++) id[i][i] = 1.0;
        Sigma_=LUP_->solve(id);
    }
  }

  double MultivariateFNormalSufficient::trace_PS() const {
      double trace=0;
      for (int i=0; i<M_; i++){
          for (int j=0; j<M_; j++){
              trace += P_[i][j]*S_[i][j];
          }
      }
      return trace;
  }

  void MultivariateFNormalSufficient::compute_sufficient_statistics_matrix()
{
    //std::cout << "computing matrix with N= " << N_ << " and M= " << M_ << std::endl;
    epsilon_ = algebra::internal::TNT::Array2D<double>(N_,M_); // deviations matrix
    for (int i=0; i<N_; i++){
        for (int j=0; j<M_; j++){
            epsilon_[i][j] = FA_[i][j] - FM_[j];
            //std::cout << "epsilon " << i << " " << j << " " << epsilon_[i][j] << std::endl;
        }
    }
    S_ = matmult(transpose(epsilon_),epsilon_);

    //for (int i=0; i<M_; i++){
    //    for (int j=0; j<M_; j++){
    //        std::cout << "S " << i << " " << j << " " << S_[i][j] << std::endl;
    //    }
    //}
}

IMPISD_END_NAMESPACE

