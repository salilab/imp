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

MultivariateFNormalSufficient::MultivariateFNormalSufficient( 
        MatrixXd FX, double JF, VectorXd FM, MatrixXd Sigma) :
    Object("Multivariate Normal distribution %1%")
{
        N_=FX.rows();
        M_=FX.cols(); 
        IMP_LOG(TERSE, "MVN: direct init with N=" << N_ 
                << " and M=" << M_ << std::endl);
        IMP_USAGE_CHECK( N_ > 0, 
            "please provide at least one observation per dimension");
        IMP_USAGE_CHECK( M_ > 0,
            "please provide at least one variable");
        FM_=FM;
        set_W_nonzero(false);
        set_FX(FX); //also computes W, Fbar and epsilon.
        set_JF(JF);
        set_Sigma(Sigma); //computes the Cholesky decomp.
}

MultivariateFNormalSufficient::MultivariateFNormalSufficient(
        VectorXd Fbar, double JF, VectorXd FM, int Nobs,  MatrixXd W,
        MatrixXd Sigma) : Object("Multivariate Normal distribution %1%")
{
        N_=Nobs;
        M_=Fbar.rows();
        IMP_LOG(TERSE, "MVN: sufficient statistics init with N=" << N_ 
                << " and M=" << M_ << std::endl);
        IMP_USAGE_CHECK( N_ > 0, 
            "please provide at least one observation per dimension");
        IMP_USAGE_CHECK( M_ > 0,
            "please provide at least one variable");
        FM_=FM;
        W_is_diagonal_ = false;
        W_is_zero_ = false;
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

VectorXd MultivariateFNormalSufficient::evaluate_derivative_FM() const
{ 
      // d(-log(p))/d(FM) = - N * P * epsilon
      IMP_LOG(TERSE, "MVN: evaluate_derivative_FM() = " << std::endl);
      return -N_ * P_ * epsilon_;
}

  MatrixXd MultivariateFNormalSufficient::evaluate_derivative_Sigma() const
  { 
      //d(-log(p))/dSigma = 1/2 (N P - N P epsilon transpose(epsilon) P - P W P)
      IMP_LOG(TERSE, "MVN: evaluate_derivative_Sigma() = " << std::endl);
      MatrixXd ptp(compute_PTP());  //O(M^2)
      MatrixXd pwp(compute_PWP()); //O(M^4), can be easily optimized
      MatrixXd R(M_,M_);
      return 0.5*(N_*(P_-ptp)-pwp);

  }
  
  void MultivariateFNormalSufficient::set_FX(MatrixXd FX) 
  {
    if (FX.rows() != FX_.rows() || FX.cols() != FX_.cols() || FX != FX_){
        if (FX.rows() != N_) {
            IMP_THROW("size mismatch for FX in the number of repetitions: got " 
                    << FX.rows() << " instead of "<<N_, ModelException);
            }
        if (FX.cols() != M_) {
            IMP_THROW("size mismatch for FX in the number of variables: got " 
                    <<FX.cols() << " instead of "<<M_, ModelException);
            }
        FX_=FX;
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

  void MultivariateFNormalSufficient::set_FM(VectorXd FM) 
  {
    if (FM.rows() != FM_.rows() || FM.cols() != FM_.cols() || FM != FM_){
        if (FM.rows() != M_) {
            IMP_THROW("size mismatch for FM: got "
                    <<FM.rows() << " instead of " << M_, ModelException);
            }
        FM_=FM;
        IMP_LOG(TERSE, "MVN:   set FM to new vector" << std::endl);
        compute_epsilon();
    }
  }

  void MultivariateFNormalSufficient::set_Fbar(VectorXd Fbar) 
  {
    if (Fbar.rows() != Fbar_.rows() || Fbar.cols() != Fbar_.cols() 
            || Fbar != Fbar_){
        if (Fbar.rows() != M_) {
            IMP_THROW("size mismatch for Fbar: got "
                    << Fbar.rows() << " instead of " << M_, ModelException);
            }
        Fbar_=Fbar;
        IMP_LOG(TERSE, "MVN:   set Fbar to new vector" << std::endl);
        compute_epsilon();
    }
  }

  void MultivariateFNormalSufficient::set_Sigma(MatrixXd Sigma)  
  {
    if (Sigma.rows() != Sigma_.rows() || Sigma.cols() != Sigma_.cols() 
            || Sigma != Sigma_){
        if (Sigma.cols() != Sigma.rows()) {
            IMP_THROW("need a square matrix!", ModelException);
            }
        Sigma_=Sigma;
        //std::cout << "Sigma: " << Sigma_ << std::endl << std::endl;
        IMP_LOG(TERSE, "MVN:   set Sigma to new matrix" << std::endl);
        IMP_LOG(TERSE, "MVN:   computing Cholesky decomposition" << std::endl);
        // compute Cholesky decomposition for determinant and inverse
        Eigen::LDLT<MatrixXd, Eigen::Upper> ldlt;
        ldlt.compute(Sigma_);
        if (!ldlt.isPositive())
            IMP_THROW("Sigma matrix is not positive semidefinite!", 
                    ModelException);
        // determinant and derived constants
        MatrixXd L(ldlt.matrixU());
        //std::cout << "L: " << L << std::endl << std::endl;
        //std::cout << "D: " << ldlt.vectorD() << std::endl << std::endl;
        double logDetSigma=2*L.diagonal().array().log().sum() 
                           + ldlt.vectorD().array().log().sum();
        IMP_LOG(TERSE, "MVN:   det(Sigma) = " << exp(logDetSigma) << std::endl);
        norm_=pow(2*IMP::PI, -double(N_*M_)/2.0) 
                    * exp(-double(N_)/2.0*logDetSigma);
        lnorm_=double(N_*M_)/2 * log(2*IMP::PI) + double(N_)/2 * logDetSigma;
        IMP_LOG(TERSE, "MVN:   norm = " << norm_ << " lnorm = " 
                << lnorm_ << std::endl);
        //inverse
        IMP_LOG(TERSE, "MVN:   solving for inverse" << std::endl);
        P_=ldlt.solve(MatrixXd::Identity(M_,M_));
        //std::cout << "P: " << P_ << std::endl << std::endl;
        ////WP
        //IMP_LOG(TERSE, "MVN:   solving for WP" << std::endl);
        //MatrixXd WP_(M_, M_);
        //WP_=CholeskySigma_->solve(W_);
        IMP_LOG(TERSE, "MVN:   done" << std::endl);
    }
  }

  void MultivariateFNormalSufficient::set_W_nonzero(bool yes, double val)
{
    if (!yes)
    {
        W_is_diagonal_ = false;
        W_is_zero_ = false;
    } else {
        W_is_diagonal_ = true;
        W_is_zero_ = true;
        for (int i=0; i<M_; i++)
        {
            for (int j=0; j<M_; j++)
            {
                if (std::abs(W_(i,j)) > val)
                {
                    if ((i!=j) && W_is_diagonal_) W_is_diagonal_ = false;
                    if (W_is_zero_) W_is_zero_ = false;
                }
            }
        }
    }
    IMP_LOG(TERSE, "W is diagonal: " << W_is_diagonal_ << std::endl);
    IMP_LOG(TERSE, "W is zero: " << W_is_zero_ << std::endl);
}

  double MultivariateFNormalSufficient::trace_WP() const 
  {
      double trace=0;
      if (!W_is_zero_)
      {
          if (W_is_diagonal_)
          {
              trace=W_.diagonal().transpose()*P_.diagonal();
          } else {
              trace = (W_.array() * P_.array()).sum();
          }
      }
      IMP_LOG(TERSE, "MVN:   trace(WP) = " << trace << std::endl);
      return trace;
  }
 
  double MultivariateFNormalSufficient::mean_dist() const
{
    //std::cout << "P " << std::endl << P_ << std::endl;
    //std::cout << "epsilon " << std::endl << epsilon_ << std::endl;
    double dist = epsilon_.transpose()*P_*epsilon_;
    IMP_LOG(TERSE, "MVN:   mean_dist = " << dist << std::endl);
    return dist;
}

  MatrixXd MultivariateFNormalSufficient::compute_PTP() const 
{
  IMP_LOG(TERSE, "MVN:   computing PTP" << std::endl);
  VectorXd peps(M_);
  peps = P_*epsilon_;
  return peps*peps.transpose();
}

MatrixXd MultivariateFNormalSufficient::compute_PWP() const
{
      //compute PWP
      IMP_LOG(TERSE, "MVN:   computing PWP" << std::endl);
      MatrixXd R(MatrixXd::Zero(M_,M_));
      if (!W_is_zero_) {
          if (W_is_diagonal_)
          {
              R = P_*W_.diagonal().asDiagonal()*P_;
          } else {
              R = P_*W_*P_;
          }
      }
      IMP_LOG(TERSE, "MVN:   done" << std::endl);
      return R;
}

  void MultivariateFNormalSufficient::compute_sufficient_statistics()
{
    IMP_LOG(TERSE, "MVN:   computing sufficient statistics" << std::endl);
    Fbar_ = FX_.colwise().mean();
    compute_epsilon();
    //
    MatrixXd A(N_,M_);
    A = FX_.rowwise() - Fbar_.transpose();
    W_ = A.transpose()*A;
    IMP_LOG(TERSE, "MVN:   done sufficient statistics" << std::endl);
}

void MultivariateFNormalSufficient::compute_epsilon()
{
    IMP_LOG(TERSE, "MVN:      computing epsilon" << std::endl);
    epsilon_ = Fbar_ - FM_;
    IMP_LOG(TERSE, "MVN:      done epsilon" << std::endl);
}

IMPISD_END_NAMESPACE

