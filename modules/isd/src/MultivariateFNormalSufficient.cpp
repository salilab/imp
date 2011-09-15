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
#include <IMP/algebra/internal/jama_cholesky.h>
#include <boost/scoped_ptr.hpp>

IMPISD_BEGIN_NAMESPACE

using IMP::algebra::internal::TNT::Array1D;
using IMP::algebra::internal::TNT::Array2D;

MultivariateFNormalSufficient::MultivariateFNormalSufficient( Array2D<double>
        FX, double JF, Array1D<double> FM, Array2D<double> Sigma) :
    Object("Multivariate Normal distribution %1%")
{
        N_=FX.dim1();
        M_=FX.dim2(); 
        IMP_LOG(TERSE, "MVN: direct init with N=" << N_ 
                << " and M=" << M_ << std::endl);
        IMP_USAGE_CHECK( N_ > 0, 
            "please provide at least one observation per dimension");
        IMP_USAGE_CHECK( M_ > 0,
            "please provide at least one variable");
        FM_=FM.copy();
        set_FX(FX); //also computes W, Fbar and epsilon.
        set_JF(JF);
        set_Sigma(Sigma); //computes the Cholesky decomp.
}

MultivariateFNormalSufficient::MultivariateFNormalSufficient(Array1D<double>
        Fbar, double JF, Array1D<double> FM, int Nobs,  Array2D<double> W,
        Array2D<double> Sigma) : Object("Multivariate Normal distribution %1%")
{
        N_=Nobs;
        M_=Fbar.dim1();
        IMP_LOG(TERSE, "MVN: sufficient statistics init with N=" << N_ 
                << " and M=" << M_ << std::endl);
        IMP_USAGE_CHECK( N_ > 0, 
            "please provide at least one observation per dimension");
        IMP_USAGE_CHECK( M_ > 0,
            "please provide at least one variable");
        FM_=FM.copy();
        set_Fbar(Fbar); //also computes epsilon
        set_W(W);
        set_JF(JF);
        set_Sigma(Sigma);
}


  /* probability density function */
double MultivariateFNormalSufficient::density() const
  { 
      double d = norm_*JF_*exp(-0.5*(trace_WP() + N_ * mean_dist()));
      IMP_LOG(TERSE, "MVN: density() = " << d << std::endl);
      return d;
  }
 
  /* energy (score) functions, aka -log(p) */
double MultivariateFNormalSufficient::evaluate() const 
  { 
      //std::cout << " mean " << double(N_)*mean_dist();
      //std::cout << " WP " << trace_WP();
      double e = lnorm_ + lJF_ + 0.5*( trace_WP() + double(N_)*mean_dist()) ;
      IMP_LOG(TERSE, "MVN: evaluate() = " << e << std::endl);
      return e;
  }

Array1D<double> MultivariateFNormalSufficient::evaluate_derivative_FM() const
{ 
      // d(-log(p))/d(FM) = - N * P * epsilon
      IMP_LOG(TERSE, "MVN: evaluate_derivative_FM() = " << std::endl);
      Array1D<double> retval(M_);
      for (int i=0; i<M_; i++) {
          retval[i]=0.0;
          for (int j=0; j<M_; j++) {
              retval[i] += - N_*P_[i][j]*epsilon_[j];
          }
      }
      return retval;
}

  Array2D<double> MultivariateFNormalSufficient::evaluate_derivative_Sigma() const
  { 
      //d(-log(p))/dSigma = 1/2 (N P - N P epsilon transpose(epsilon) P - P W P)
      IMP_LOG(TERSE, "MVN: evaluate_derivative_Sigma() = " << std::endl);
      Array2D<double> ptp(compute_PTP());  //O(M^2)
      Array2D<double> pwp(compute_PWP()); //O(M^4), can be easily optimized
      Array2D<double> R(M_,M_);
      for (int i=0; i<M_; i++){
          for (int j=0; j<M_; j++){
              R[i][j] = 0.5*(N_*(P_[i][j] - ptp[i][j]) - pwp[i][j]);
          }
      }
      return R;

  }
  
  bool MultivariateFNormalSufficient::are_equal(Array1D<double> A,
                 Array1D<double> B) const 
   {
      if (A.dim1() != B.dim1()) return false;
      for (int i=0; i<A.dim1(); i++){
              if (A[i] != B[i]) return false;
      }
      return true;
  }

  bool MultivariateFNormalSufficient::are_equal(Array2D<double> A,
                 Array2D<double> B) const 
   {
      if (A.dim1() != B.dim1()) return false;
      if (A.dim2() != B.dim2()) return false;
      for (int i=0; i<A.dim1(); i++){
          for (int j=0; j<A.dim2(); j++){
              if (A[i][j] != B[i][j]) return false;
          }
      }
      return true;
  }

  void MultivariateFNormalSufficient::set_FX(Array2D<double> FX) 
  {
    if (!are_equal(FX,FX_)){
        if (FX.dim1() != N_) {
            IMP_THROW("size mismatch for FX in the number of repetitions: got " 
                    << FX.dim1() << " instead of "<<N_, ModelException);
            }
        if (FX.dim2() != M_) {
            IMP_THROW("size mismatch for FX in the number of variables: got " 
                    <<FX.dim2() << " instead of "<<M_, ModelException);
            }
        FX_=FX.copy();
        IMP_LOG(TERSE, "MVN:   set FX to new matrix"<< std::endl);
        compute_sufficient_statistics();
    }
  }

  void MultivariateFNormalSufficient::set_JF(double f) 
  {
    JF_=f;
    lJF_=-log(JF_);
    IMP_LOG(TERSE, "MVN:   set JF = " << JF_ << " lJF_ = " << lJF_ <<std::endl);
  }

  void MultivariateFNormalSufficient::set_FM(Array1D<double> FM) 
  {
    if (!are_equal(FM,FM_)){
        if (FM.dim1() != M_) {
            IMP_THROW("size mismatch for FM: got "
                    <<FM.dim1() << " instead of " << M_, ModelException);
            }
        FM_=FM.copy();
        IMP_LOG(TERSE, "MVN:   set FM to new vector" << std::endl);
        compute_epsilon();
    }
  }

  void MultivariateFNormalSufficient::set_Fbar(Array1D<double> Fbar) 
  {
    if (!are_equal(Fbar,Fbar_)){
        if (Fbar.dim1() != M_) {
            IMP_THROW("size mismatch for Fbar: got "
                    << Fbar.dim1() << " instead of " << M_, ModelException);
            }
        Fbar_=Fbar.copy();
        IMP_LOG(TERSE, "MVN:   set Fbar to new vector" << std::endl);
        compute_epsilon();
    }
  }

  void MultivariateFNormalSufficient::set_Sigma(Array2D<double> Sigma)  
  {
    if (!are_equal(Sigma,Sigma_)) {
        if (Sigma.dim2() != Sigma.dim1()) {
            IMP_THROW("need a square matrix!", ModelException);
            }
        Sigma_=Sigma;
        IMP_LOG(TERSE, "MVN:   set Sigma to new matrix" << std::endl);
        IMP_LOG(TERSE, "MVN:   computing Cholesky decomposition" << std::endl);
        // compute Cholesky decomposition for determinant and inverse
        CholeskySigma_.reset(new algebra::internal::JAMA::Cholesky<double> 
                (Sigma_));
        if (!CholeskySigma_->is_spd())
            IMP_THROW("Sigma matrix is not symmetric positive definite!", 
                    ModelException);
        // determinant and derived constants
        double logDetSigma=0;
        Array2D<double> L(CholeskySigma_->getL());
        for (int i=0; i<M_; i++) logDetSigma += std::log(L[i][i]);
        logDetSigma *= 2;
        IMP_LOG(TERSE, "MVN:   det(Sigma) = " << exp(logDetSigma) << std::endl);
        norm_=pow(2*IMP::PI, -double(N_*M_)/2.0) 
                    * exp(-double(N_)/2.0*logDetSigma);
        lnorm_=double(N_*M_)/2 * log(2*IMP::PI) + double(N_)/2 * logDetSigma;
        IMP_LOG(TERSE, "MVN:   norm = " << norm_ << " lnorm = " 
                << lnorm_ << std::endl);
        //inverse (taken from TNT website)
        IMP_LOG(TERSE, "MVN:   solving for inverse" << std::endl);
        Array2D<double> id(M_, M_, 0.0);
        for (int i=0; i<M_; i++) id[i][i] = 1.0;
        P_=CholeskySigma_->solve(id);
        IMP_LOG(TERSE, "MVN:   done" << std::endl);
    }
  }

  double MultivariateFNormalSufficient::trace_WP() const 
  {
      double trace=0;
      for (int i=0; i<M_; i++){
          for (int j=0; j<M_; j++){
              trace += W_[i][j]*P_[i][j];
          }
      }
      IMP_LOG(TERSE, "MVN:   trace(WP) = " << trace << std::endl);
      return trace;
  }
 
  double MultivariateFNormalSufficient::mean_dist() const
{
    double dist=0;
    for (int i=0; i<M_; i++){
        for (int j=0; j<M_; j++){
            dist += epsilon_[i]*P_[i][j]*epsilon_[j];
        }
    }
    IMP_LOG(TERSE, "MVN:   mean_dist = " << dist << std::endl);
    return dist;
}

  Array2D<double> MultivariateFNormalSufficient::compute_PTP() const 
{
  //compute P*epsilon
  IMP_LOG(TERSE, "MVN:   computing PTP" << std::endl);
  Array1D<double> peps(M_);
  for (int i=0; i<M_; i++){
      peps[i] = 0.0;
      for (int j=0; j<M_; j++){
          peps[i] += P_[i][j]*epsilon_[j];
      }
  }
  //compute peps*trans(peps)
  Array2D<double> R(M_,M_);
  for (int i=0; i<M_; i++){
      for (int j=0; j<M_; j++){
          R[i][j] = peps[i]*peps[j];
      }
  }
  IMP_LOG(TERSE, "MVN:   done" << std::endl);
  return R;
}

Array2D<double> MultivariateFNormalSufficient::compute_PWP() const
{
      //compute PWP
      IMP_LOG(TERSE, "MVN:   computing PWP" << std::endl);
      Array2D<double> R(M_,M_);
      Array2D<double> WP(M_,M_);
      for (int k=0; k<M_; k++){
        for (int j=0; j<M_; j++){
            WP[k][j] = 0.0;
            for (int l=0; l<M_; l++){
                WP[k][j] += W_[k][l]*P_[l][j];
            }
        }
      }
      for (int i=0; i<M_; i++){
        for (int j=0; j<M_; j++){
            R[i][j] = 0.0;
            for (int k=0; k<M_; k++){
                R[i][j] += P_[i][k]*WP[k][j];
            }
        }
      }
      IMP_LOG(TERSE, "MVN:   done" << std::endl);
      return R;
}

  void MultivariateFNormalSufficient::compute_sufficient_statistics()
{
    IMP_LOG(TERSE, "MVN:   computing sufficient statistics" << std::endl);
    Fbar_ = Array1D<double> (M_,0.0);
    for (int j=0; j<M_; j++){
        for (int i=0; i<N_; i++){
            Fbar_[j] += FX_[i][j];
        }
        Fbar_[j] = Fbar_[j]/N_;
    }
    compute_epsilon();
    //
    W_ = Array2D<double> (M_,M_,0.0);
    for (int k=0; k<N_; k++){
        for (int i=0; i<M_; i++){
            double aik = FX_[k][i] - Fbar_[i];
            for (int j=0; j<=i; j++){
                double ajk=FX_[k][j] - Fbar_[j];
                W_[i][j] += aik*ajk;
            }
        }
    }
    for (int i=0; i<M_; i++){
        for (int j=i+1; j<M_; j++){
            W_[i][j] = W_[j][i];
        }
    }
    IMP_LOG(TERSE, "MVN:   done sufficient statistics" << std::endl);
}

void MultivariateFNormalSufficient::compute_epsilon()
{
    IMP_LOG(TERSE, "MVN:      computing epsilon" << std::endl);
    epsilon_ = Array1D<double> (M_);
    for (int i=0; i<M_; i++){
        epsilon_[i] = Fbar_[i] - FM_[i];
    }
    IMP_LOG(TERSE, "MVN:      done epsilon" << std::endl);
}

IMPISD_END_NAMESPACE

