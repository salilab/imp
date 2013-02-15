/**
 *  \file MultivariateFNormalSufficient.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/MultivariateFNormalSufficient.h>
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/constants.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

//should have IMP_MVN_TIMER_NFUNCS func_names
enum func_names { EVAL, TRWP, MD, DFM, DSIGMA, PTP, PWP, CHOLESKY, SOLVE,
    PW_DIRECT, PW_CG_SUCCESS };

MultivariateFNormalSufficient::MultivariateFNormalSufficient(
        const MatrixXd& FX, double JF, const VectorXd& FM,
        const MatrixXd& Sigma, double factor) :
  base::Object("Multivariate Normal distribution %1%")
{
        //O(1)
        reset_flags();
        N_=FX.rows();
        M_=FX.cols();
        IMP_LOG_TERSE( "MVN: direct init with N=" << N_
                << " and M=" << M_ << std::endl);
        IMP_USAGE_CHECK( N_ > 0,
            "please provide at least one observation per dimension");
        IMP_USAGE_CHECK( M_ > 0,
            "please provide at least one variable");
        set_factor(factor);
        set_FM(FM);
        set_FX(FX);
        set_jacobian(JF);
        set_Sigma(Sigma);
        use_cg_=false;
}

MultivariateFNormalSufficient::MultivariateFNormalSufficient(
        const VectorXd& Fbar, double JF, const VectorXd& FM, int Nobs,
        const MatrixXd& W, const MatrixXd& Sigma, double factor)
  : base::Object("Multivariate Normal distribution %1%")
{
        reset_flags();
        N_=Nobs;
        M_=Fbar.rows();
        IMP_LOG_TERSE( "MVN: sufficient statistics init with N=" << N_
                << " and M=" << M_ << std::endl);
        IMP_USAGE_CHECK( N_ > 0,
            "please provide at least one observation per dimension");
        IMP_USAGE_CHECK( M_ > 0,
            "please provide at least one variable");
        set_factor(factor);
        set_FM(FM);
        set_Fbar(Fbar);
        set_W(W);
        set_jacobian(JF);
        set_Sigma(Sigma);
        use_cg_=false;
}

void MultivariateFNormalSufficient::setup_cg()
{
        cg_ = new internal::ConjugateGradientEigen();
        precond_ = MatrixXd::Identity(M_,M_);
        first_PW_ = true;
        first_PWP_ = true;
}

void MultivariateFNormalSufficient::set_use_cg(bool use, double tol)
 {
     use_cg_=use;
     cg_tol_ = std::abs(tol);
     if (use) setup_cg();
 }

void MultivariateFNormalSufficient::stats() const
{
    static std::string func_displays[IMP_MVN_TIMER_NFUNCS] =
      { "eval/density", "  trace(WP)",
          "  get_mean_square_residuals", "deriv_FM", "deriv_Sigma",
          "  compute_PTP", "  compute_PWP", "Cholesky", "Sigma*X=B",
          "compute_PW_direct", "compute_PW_CG_success"};
    timer_.stats(func_displays);
}

void MultivariateFNormalSufficient::reset_flags()
{
  flag_FM_ = false;
  flag_FX_ = false;
  flag_Fbar_ = false;
  flag_W_ = false;
  flag_Sigma_ = false;
  flag_epsilon_ = false;
  flag_PW_ = false;
  flag_P_ = false;
  flag_ldlt_ = false;
  flag_norms_ = false;
  flag_Peps_ = false;
}

  /* probability density function */
double MultivariateFNormalSufficient::density() const
  {
      timer_.start(EVAL);
      double d;
      if (N_ == 1)
      {
          d=get_norms()[0]*get_jacobian()
              *exp(-0.5*get_mean_square_residuals()/IMP::square(factor_));
      } else {
          d = get_norms()[0]*get_jacobian()
          *exp(-0.5*(trace_WP()
                     + N_ * get_mean_square_residuals())/IMP::square(factor_));
      }
      IMP_LOG_TERSE( "MVN: density() = " << d << std::endl);
      timer_.stop(EVAL);
      return d;
  }

  /* energy (score) functions, aka -log(p).
   * O(M) if Sigma is up to date
   * O(M^3) if Sigma has changed (Cholesky decomp.) */
double MultivariateFNormalSufficient::evaluate() const
  {
      timer_.start(EVAL);
      double e;
      if (N_==1)
      {
          e = get_norms()[1] + get_minus_log_jacobian()
          + 0.5*get_mean_square_residuals()/IMP::square(factor_) ;
          /*std::cout << "mvn"
              << " " << e
              << " " << get_norms()[0]
              << " " << get_norms()[1]
              << " " << get_minus_log_jacobian()
              << " " << get_mean_square_residuals()/IMP::square(factor_)
              <<std::endl; */
      } else {
          e = get_norms()[1] + get_minus_log_jacobian()
          + 0.5*( trace_WP()
                + double(N_)*get_mean_square_residuals())/IMP::square(factor_) ;
      }
      IMP_LOG_TERSE( "MVN: evaluate() = " << e << std::endl);
      timer_.stop(EVAL);
      return e;
  }

double MultivariateFNormalSufficient::get_minus_exponent() const
{
    double e;
    if (N_ == 1)
    {
        e = 0.5*get_mean_square_residuals()/IMP::square(factor_) ;
    } else {
        e = 0.5*( trace_WP()
                + double(N_)*get_mean_square_residuals())/IMP::square(factor_) ;
    }
    return e;
}

double MultivariateFNormalSufficient::get_minus_log_normalization() const
{
    double e = get_norms()[1] + get_minus_log_jacobian();
    return e;
}


  // O(1) if up to date, O(M^2) if epsilon new, O(M^3) if Sigma new
VectorXd MultivariateFNormalSufficient::evaluate_derivative_FM() const
{
      timer_.start(DFM);
      // d(-log(p))/d(FM) = - N * P * epsilon
      IMP_LOG_TERSE( "MVN: evaluate_derivative_FM() = " << std::endl);
      VectorXd tmp(-N_ * get_Peps()/IMP::square(factor_));
      timer_.stop(DFM);
      return tmp;
}

  MatrixXd MultivariateFNormalSufficient::evaluate_derivative_Sigma() const
  {
      timer_.start(DSIGMA);
      //d(-log(p))/dSigma = 1/2 (N P - N P epsilon transpose(epsilon) P - PWP)
      IMP_LOG_TERSE( "MVN: evaluate_derivative_Sigma() = " << std::endl);
      MatrixXd R;
      if (N_==1) // O(M) if up to date, O(M^3) if Sigma new
      {
          R = 0.5*(get_P()-compute_PTP()/IMP::square(factor_));
      } else { // O(M^2) if up to date, O(M^3) if Sigma new
          double f2=IMP::square(factor_);
          R = 0.5*(N_*(get_P()-compute_PTP()/f2)-compute_PWP()/f2);
      }
      timer_.stop(DSIGMA);
      return R;
  }

  double MultivariateFNormalSufficient::evaluate_derivative_factor() const
  {
      //warning: untested!
      //d(-log(p))/dfactor = -N/f^3 trans(eps)P(eps) -1/f^3 tr(WP) + NM/f
      IMP_LOG_TERSE( "MVN: evaluate_derivative_factor() = " << std::endl);
      double R;
      if (N_==1)
      {
          R = - get_mean_square_residuals()/IMP::cube(factor_)
              + double(M_)/factor_;
      } else {
          R = - (double(N_)*get_mean_square_residuals()
                  + trace_WP())/IMP::cube(factor_)
              + double(M_*N_)/factor_;
      }
      return R;
  }

  MatrixXd MultivariateFNormalSufficient::evaluate_second_derivative_FM_FM()
    const
  {
      if (N_!=1) IMP_THROW("not implemented when N>1", ModelException);
      MatrixXd ret(get_P()/IMP::square(factor_));
      return ret;
  }

  MatrixXd MultivariateFNormalSufficient::evaluate_second_derivative_FM_Sigma(
          unsigned k) const
  {
      if (N_!=1) IMP_THROW("not implemented when N>1", ModelException);
      MatrixXd P(get_P());
      VectorXd Peps(get_Peps());
      MatrixXd ret(P.transpose().col(k)*Peps.transpose());
      return ret/IMP::square(factor_);
  }

MatrixXd MultivariateFNormalSufficient::evaluate_second_derivative_Sigma_Sigma(
        unsigned m, unsigned n) const
  {
      if (N_!=1) IMP_THROW("not implemented when N>1", ModelException);
      MatrixXd P(get_P());
      VectorXd Peps(get_Peps());
      MatrixXd tmp(P.col(m)*Peps.transpose());
      MatrixXd ret(0.5*(-P.col(n)*P.row(m)
                        +Peps(n)*(tmp+tmp.transpose())));
      return ret/IMP::square(factor_);
  }

  MatrixXd MultivariateFNormalSufficient::get_FX() const
{
    if (!flag_FX_)
    {
        IMP_THROW("FX was not set!", ModelException);
    }
    return FX_;
}

  void MultivariateFNormalSufficient::set_FX(const MatrixXd& FX)
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
        IMP_LOG_TERSE( "MVN:   set FX to new matrix"<< std::endl);
        flag_Fbar_ = false;
        flag_epsilon_ = false;
        flag_W_ = false;
        flag_PW_ = false;
        flag_Peps_ = false;
    }
    flag_FX_ = true;
  }

  VectorXd MultivariateFNormalSufficient::get_FM() const
{
    if (!flag_FM_)
    {
        IMP_THROW("FM was not set!", ModelException);
    }
    return FM_;
}

  void MultivariateFNormalSufficient::set_FM(const VectorXd& FM)
  {
    if (FM.rows() != FM_.rows() || FM.cols() != FM_.cols() || FM != FM_){
        if (FM.rows() != M_) {
            IMP_THROW("size mismatch for FM: got "
                    <<FM.rows() << " instead of " << M_, ModelException);
            }
        FM_=FM;
        IMP_LOG_TERSE( "MVN:   set FM to new vector" << std::endl);
        flag_epsilon_ = false;
        flag_Peps_ = false;
    }
    flag_FM_ = true;
  }

  VectorXd MultivariateFNormalSufficient::get_Fbar() const
{
    if (!flag_Fbar_)
    {
        // Fbar is either given or computed from FX
        // try to get FX which will throw an exception if it is not set
        VectorXd Fbar = get_FX().colwise().mean();
        const_cast<MultivariateFNormalSufficient *>(this)->set_Fbar(Fbar);
    }
    return Fbar_;
}

  void MultivariateFNormalSufficient::set_Fbar(const VectorXd& Fbar)
  {
    if (Fbar.rows() != Fbar_.rows() || Fbar.cols() != Fbar_.cols()
            || Fbar != Fbar_){
        if (Fbar.rows() != M_) {
            IMP_THROW("size mismatch for Fbar: got "
                    << Fbar.rows() << " instead of " << M_, ModelException);
            }
        Fbar_=Fbar;
        IMP_LOG_TERSE( "MVN:   set Fbar to new vector" << std::endl);
        flag_epsilon_ = false;
        flag_W_ = false;
        flag_PW_ = false;
        flag_Peps_ = false;
    }
    flag_Fbar_ = true;
  }

  double MultivariateFNormalSufficient::get_jacobian() const
{
    return JF_;
}

  void MultivariateFNormalSufficient::set_jacobian(double JF)
  {
        JF_=JF;
        lJF_=-log(JF_);
  }

  double MultivariateFNormalSufficient::get_minus_log_jacobian() const
{
    return lJF_;
}

  void MultivariateFNormalSufficient::set_minus_log_jacobian(double lJF)
  {
        JF_=exp(-lJF);
        lJF_=lJF;
  }

  VectorXd MultivariateFNormalSufficient::get_epsilon() const
{
    if (!flag_epsilon_)
    {
        VectorXd epsilon = get_Fbar() - get_FM();
        const_cast<MultivariateFNormalSufficient*>(this)->set_epsilon(epsilon);
    }
    return epsilon_;
}

  void MultivariateFNormalSufficient::set_epsilon(const VectorXd& eps)
{
    IMP_LOG_TERSE( "MVN:   set epsilon to new vector"<< std::endl);
    epsilon_ = eps;
    flag_epsilon_ = true;
    flag_Peps_ = false;
}

  MatrixXd MultivariateFNormalSufficient::get_W() const
{
    if (!flag_W_)
    {
        //this only happens if W was not given as input
        //compute W from epsilon
        IMP_LOG_TERSE( "MVN:   computing W" << std::endl);
        MatrixXd W(M_,M_);
        if (N_ == 1)
        {
            W.setZero();
        } else {
            MatrixXd A(get_FX().rowwise() - get_Fbar().transpose());
            W = A.transpose()*A;
        }
        const_cast<MultivariateFNormalSufficient *>(this)->set_W(W);
    }
    return W_;
}

  void MultivariateFNormalSufficient::set_W(const MatrixXd& W)
{
    if (W.rows() != W_.rows() || W.cols() != W_.cols() || W != W_)
    {
        if (W.cols() != W.rows()) {
            IMP_THROW("need a square matrix!", ModelException);
            }
        if (W.rows() != M_) {
            IMP_THROW("size mismatch for W!" , ModelException);
            }
        W_ = W;
        IMP_LOG_TERSE( "MVN:   set W to new matrix"<< std::endl);
        flag_PW_ = false;
    }
    flag_W_ = true;
}

  MatrixXd MultivariateFNormalSufficient::get_Sigma() const
{
    if (!flag_Sigma_)
    {
        IMP_THROW("Sigma was not set!", ModelException);
    }
    return Sigma_;
}

  void MultivariateFNormalSufficient::set_Sigma(const MatrixXd& Sigma)
  {
    if (Sigma.rows() != Sigma_.rows() || Sigma.cols() != Sigma_.cols()
            || Sigma != Sigma_){
        if (Sigma.cols() != Sigma.rows()) {
            IMP_THROW("need a square matrix!", ModelException);
            }
        Sigma_=Sigma;
        //std::cout << "Sigma: " << Sigma_ << std::endl << std::endl;
        IMP_LOG_TERSE( "MVN:   set Sigma to new matrix" << std::endl);
        flag_ldlt_ = false;
        flag_P_ = false;
        flag_PW_ = false;
        flag_norms_ = false;
        flag_Peps_ = false;
    }
    flag_Sigma_ = true;
  }

  double MultivariateFNormalSufficient::get_factor() const
{
    return factor_;
}

  void MultivariateFNormalSufficient::set_factor(double f)
{
    factor_=f;
    flag_norms_ = false;
}

//Eigen::LLT<MatrixXd, Eigen::Upper>
Eigen::LDLT<MatrixXd, Eigen::Upper>
MultivariateFNormalSufficient::get_ldlt() const
{
    if (!flag_ldlt_)
    {
        timer_.start(CHOLESKY);
        IMP_LOG_TERSE( "MVN:   computing Cholesky decomposition" << std::endl);
        // compute Cholesky decomposition for determinant and inverse
        //Eigen::LLT<MatrixXd, Eigen::Upper> ldlt(get_Sigma());
        Eigen::LDLT<MatrixXd, Eigen::Upper> ldlt(get_Sigma());
        //if (ldlt.info() != Eigen::Success)
        if (!ldlt.isPositive())
        {
            std::cout << "Sigma" << std::endl;
            std::cout << get_Sigma() << std::endl;
            IMP_THROW("Sigma matrix is not positive semidefinite!",
                    ModelException);
        }
        const_cast<MultivariateFNormalSufficient *>(this)->set_ldlt(ldlt);
        timer_.stop(CHOLESKY);
    }
    return ldlt_;
}

void MultivariateFNormalSufficient::set_ldlt(
          const Eigen::LDLT<MatrixXd, Eigen::Upper>& ldlt)
{
    IMP_LOG_TERSE( "MVN:   set ldlt factorization"<< std::endl);
    ldlt_ = ldlt;
    flag_ldlt_ = true;
    flag_P_ = false;
    flag_PW_ = false;
    flag_norms_ = false;
    flag_Peps_ = false;
}

std::vector<double> MultivariateFNormalSufficient::get_norms() const
{
    if (!flag_norms_)
    {
        //Eigen::LLT<MatrixXd, Eigen::Upper> ldlt(get_ldlt());
        Eigen::LDLT<MatrixXd, Eigen::Upper> ldlt(get_ldlt());
        // determinant and derived constants
        //MatrixXd L(ldlt.matrixU());
        //std::cout << "L: " << L << std::endl << std::endl;
        //std::cout << "D: " << ldlt.vectorD() << std::endl << std::endl;
        //double logDetSigma=2*L.diagonal().array().log().sum() ;
        double logDetSigma = ldlt.vectorD().array().abs().log().sum();
        IMP_LOG_TERSE( "MVN:   det(Sigma) = " <<exp(logDetSigma) << std::endl);
        double norm=pow(2*IMP::PI*IMP::square(factor_), -double(N_*M_)/2.0)
                    * exp(-double(N_)/2.0*logDetSigma);
        double lnorm=double(N_*M_)/2 * log(2*IMP::PI*IMP::square(factor_))
            + double(N_)/2 * logDetSigma;
        IMP_LOG_TERSE( "MVN:   norm = " << norm << " lnorm = "
                << lnorm << std::endl);
        const_cast<MultivariateFNormalSufficient *>(this)
            ->set_norms(norm,lnorm);
    }
    std::vector<double> norms;
    norms.push_back(norm_);
    norms.push_back(lnorm_);
    return norms;
}

void MultivariateFNormalSufficient::set_norms(double norm, double lnorm)
{
    norm_ = norm;
    lnorm_ = lnorm;
    IMP_LOG_TERSE( "MVN:   set norms" << std::endl);
    flag_norms_ = true;
}

MatrixXd MultivariateFNormalSufficient::get_P() const
{
    if (!flag_P_)
    {
        //inverse
        timer_.start(SOLVE);
        //Eigen::LLT<MatrixXd, Eigen::Upper> ldlt(get_ldlt());
        Eigen::LDLT<MatrixXd, Eigen::Upper> ldlt(get_ldlt());
        IMP_LOG_TERSE( "MVN:   solving for inverse" << std::endl);
        const_cast<MultivariateFNormalSufficient *>(this)
            ->set_P(ldlt.solve(MatrixXd::Identity(M_,M_)));
        timer_.stop(SOLVE);
    }
    return P_;
}

void MultivariateFNormalSufficient::set_P(const MatrixXd& P)
{
    P_ = P;
    if (use_cg_) precond_ = P;
    //std::cout << "P: " << P_ << std::endl << std::endl;
    IMP_LOG_TERSE( "MVN:   set P to new matrix" << std::endl);
    flag_P_ = true;
}

MatrixXd MultivariateFNormalSufficient::get_PW() const
{
    if (!flag_PW_)
    {
        ////PW
        timer_.start(SOLVE);
        MatrixXd PW(M_,M_);
        if (N_==1)
        {
            IMP_LOG_TERSE( "MVN:   W=0 => PW=0" << std::endl);
            PW.setZero();
        } else {
            IMP_LOG_TERSE( "MVN:   solving for PW" << std::endl);
            if (use_cg_)
            {
                if (first_PW_)
                {
                    PW = compute_PW_direct();
                    (*const_cast<bool *>(&first_PW_))=false;
                } else {
                    PW = compute_PW_cg();
                }
            } else {
                PW = compute_PW_direct();
            }
        }
        const_cast<MultivariateFNormalSufficient *>(this)->set_PW(PW);
        timer_.stop(SOLVE);
    }
    return PW_;
}

MatrixXd MultivariateFNormalSufficient::compute_PW_direct() const
{
    timer_.start(PW_DIRECT);
    //Eigen::LLT<MatrixXd, Eigen::Upper> ldlt(get_ldlt());
    Eigen::LDLT<MatrixXd, Eigen::Upper> ldlt(get_ldlt());
    MatrixXd tmp(ldlt.solve(get_W()));
    timer_.stop(PW_DIRECT);
    return tmp;
}

MatrixXd MultivariateFNormalSufficient::compute_PW_cg() const
{
    //compute PW using CG. Preconditionner is Sigma^-1 and initial guess
    //is previous value of PW. Do M steps (theoretically sufficient) and if the
    //residuals are too big do the inversion.
    //
    timer_.start(PW_CG_SUCCESS);
    //static unsigned numtries=0;
    //static unsigned numfail=0;
    cg_->set_A(get_Sigma());
    cg_->set_B(get_W());
    cg_->set_X0(PW_);
    cg_->set_tol(cg_tol_);
    MatrixXd PW(cg_->optimize(precond_, M_));
    if (cg_->info()>0) timer_.stop(PW_CG_SUCCESS);
    double resid = (get_Sigma()*PW-get_W()).norm();
    if (resid > cg_tol_)
    {
        //numfail++;
        PW = compute_PW_direct();
    }
    //numtries++;
    //std::cout << "CG: numtries="<<numtries<<" numfail="<<numfail<<std::endl;
    return PW;
}

void MultivariateFNormalSufficient::set_PW(const MatrixXd& PW)
{
    PW_ = PW;
    //std::cout << "PW: " << PW_ << std::endl << std::endl;
    IMP_LOG_TERSE( "MVN:   set PW to new matrix" << std::endl);
    flag_PW_ = true;
}

VectorXd MultivariateFNormalSufficient::get_Peps() const
{
    if (!flag_Peps_)
    {
        ////Peps
        timer_.start(SOLVE);
        IMP_LOG_TERSE( "MVN:   solving for P*epsilon" << std::endl);
        const_cast<MultivariateFNormalSufficient *>(this)
            ->set_Peps(get_ldlt().solve(get_epsilon()));
        timer_.stop(SOLVE);
    }
    return Peps_;
}

void MultivariateFNormalSufficient::set_Peps(const VectorXd& Peps)
{
    Peps_ = Peps;
    //std::cout << "Peps: " << Peps_ << std::endl << std::endl;
    IMP_LOG_TERSE( "MVN:   set P*epsilon to new matrix" << std::endl);
    flag_Peps_ = true;
}

  double MultivariateFNormalSufficient::trace_WP() const
  {
      timer_.start(TRWP);
      double trace;
      if (N_==1)
      {
          trace=0;
      } else {
          if (use_cg_)
          {
              trace = get_PW().trace();
          } else {
              trace = (get_P()*get_W()).trace();
          }
          IMP_LOG_TERSE( "MVN:   trace(WP) = " << trace << std::endl);
      }
      timer_.stop(TRWP);
      return trace;
  }

  double MultivariateFNormalSufficient::get_mean_square_residuals() const
{
    timer_.start(MD);
    //std::cout << "P " << std::endl << P_ << std::endl;
    //std::cout << "epsilon " << std::endl << get_epsilon() << std::endl;
    VectorXd Peps(get_Peps());
    VectorXd epsilon(get_epsilon());
    double dist = epsilon.transpose()*Peps;
    IMP_LOG_TERSE( "MVN:   get_mean_square_residuals = " << dist << std::endl);
    timer_.stop(MD);
    return dist;
}

  MatrixXd MultivariateFNormalSufficient::compute_PTP() const
{
  timer_.start(PTP);
  IMP_LOG_TERSE( "MVN:   computing PTP" << std::endl);
  VectorXd peps(get_Peps());
  MatrixXd tmp(peps*peps.transpose());
  timer_.stop(PTP);
  return tmp;
}

MatrixXd MultivariateFNormalSufficient::compute_PWP() const
{
      timer_.start(PWP);
      //compute PWP
      MatrixXd tmp(M_,M_);
      if (N_==1)
      {
          IMP_LOG_TERSE( "MVN:   W = 0 => PWP = 0" << std::endl);
          tmp.setZero();
      } else {
          IMP_LOG_TERSE( "MVN:   computing PWP" << std::endl);
          /*if (use_cg_)
          {
              if (first_PW_)
              {
                  PW = compute_PW_direct();
                  (*const_cast<bool *>(&first_PW_))=false;
              } else {
                  PW = compute_PW_cg();
              }
          } else {
              PW = compute_PW_direct();
          }*/
            //if (use_cg_)
          //{
              //MatrixXd WP(get_PW().transpose());
              //return get_ldlt().solve(WP);
              //return get_P()*WP;
          //} else {
              MatrixXd P(get_P());
              MatrixXd W(get_W());
              tmp=P*W*P;
          //}
      }
      timer_.stop(PWP);
      return tmp;
}

VectorXd MultivariateFNormalSufficient::get_Sigma_eigenvalues() const
{
    Eigen::SelfAdjointEigenSolver<MatrixXd> eigensolver(get_Sigma(),
            Eigen::EigenvaluesOnly);
    if (eigensolver.info() != Eigen::Success)
            IMP_THROW("Could not determine eigenvalues!", ModelException);
    return eigensolver.eigenvalues();
}

double MultivariateFNormalSufficient::get_Sigma_condition_number() const
{
    return get_Sigma().norm()*get_P().norm();
}

MatrixXd MultivariateFNormalSufficient::solve(MatrixXd B) const
{
    return get_ldlt().solve(B);
}

IMPISD_END_NAMESPACE
