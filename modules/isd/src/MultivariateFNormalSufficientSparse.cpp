/**
 *  \file MultivariateFNormalSufficientSparse.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/MultivariateFNormalSufficientSparse.h>

#ifdef IMP_ISD_USE_CHOLMOD

#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <boost/scoped_ptr.hpp>

IMPISD_BEGIN_NAMESPACE

MultivariateFNormalSufficientSparse::MultivariateFNormalSufficientSparse(
        const MatrixXd& FX, double JF, const VectorXd& FM,
        const SparseMatrix<double>& Sigma, cholmod_common *c, double cutoff) :
    Object("Multivariate Normal distribution %1%")
{
        c_ = c;
        W_=nullptr;
        Sigma_=nullptr;
        P_=nullptr;
        PW_=nullptr;
        epsilon_=nullptr;
        L_=nullptr;
        N_=FX.rows();
        M_=FX.cols();
        IMP_LOG_TERSE( "MVNsparse: direct init with N=" << N_
                << " and M=" << M_ << std::endl);
        IMP_USAGE_CHECK( N_ > 0,
            "please provide at least one observation per dimension");
        IMP_USAGE_CHECK( M_ > 0,
            "please provide at least one variable");
        FM_=FM;
        set_FX(FX,cutoff); //also computes W, Fbar and epsilon.
        set_JF(JF);
        set_Sigma(Sigma); //computes the Cholesky decomp.
}

MultivariateFNormalSufficientSparse::MultivariateFNormalSufficientSparse(
        const VectorXd& Fbar, double JF, const VectorXd& FM, int Nobs,
        const SparseMatrix<double>& W, const SparseMatrix<double>& Sigma,
        cholmod_common *c)
        : Object("Multivariate Normal distribution %1%")
{
        c_ = c;
        W_=nullptr;
        Sigma_=nullptr;
        P_=nullptr;
        PW_=nullptr;
        epsilon_=nullptr;
        L_=nullptr;
        N_=Nobs;
        M_=Fbar.rows();
        IMP_LOG_TERSE( "MVNsparse: sufficient statistics init with N=" << N_
                << " and M=" << M_ << std::endl);
        IMP_USAGE_CHECK( N_ > 0,
            "please provide at least one observation per dimension");
        IMP_USAGE_CHECK( M_ > 0,
            "please provide at least one variable");
        FM_=FM;
        set_Fbar(Fbar); //also computes epsilon
        set_W(W);
        set_JF(JF);
        set_Sigma(Sigma);
}

  /* probability density function */
double MultivariateFNormalSufficientSparse::density() const
  {
      double d = norm_*JF_*exp(-0.5*(trace_WP() + N_ * mean_dist()));
      IMP_LOG_TERSE( "MVNsparse: density() = " << d << std::endl);
      return d;
  }

  /* energy (score) functions, aka -log(p) */
double MultivariateFNormalSufficientSparse::evaluate() const
  {
      //std::cout << " mean " << double(N_)*mean_dist();
      //std::cout << " WP " << trace_WP();
      double e = lnorm_ + lJF_ + 0.5*( trace_WP() + double(N_)*mean_dist()) ;
      IMP_LOG_TERSE( "MVNsparse: evaluate() = " << e << std::endl);
      return e;
  }

cholmod_dense *
MultivariateFNormalSufficientSparse::evaluate_derivative_FM() const
{
      // d(-log(p))/d(FM) = - N * P * epsilon
      IMP_LOG_TERSE( "MVNsparse: evaluate_derivative_FM() = " << std::endl);
      cholmod_dense *tmp = cholmod_solve(CHOLMOD_A, L_, epsilon_, c_);
      if (L_->xtype != CHOLMOD_REAL)
          IMP_THROW("matrix type is not real, update the code first",
                ModelException);
      double *x = (double *)tmp->x;
      for (size_t i=0; i<tmp->nzmax; ++i) x[i] *= -N_;
      return tmp;
}

  cholmod_sparse *
  MultivariateFNormalSufficientSparse::evaluate_derivative_Sigma() const
  {
      //d(-log(p))/dSigma = 1/2 (N P - N P epsilon transpose(epsilon) P - P W P)
      IMP_LOG_TERSE( "MVNsparse: evaluate_derivative_Sigma() = " << std::endl);
      cholmod_sparse *ptp(compute_PTP());
      cholmod_sparse *pwp(compute_PWP());
      //std::cout << " ptp " << std::endl << ptp << std::endl << std::endl;
      //std::cout << " pwp " << std::endl << pwp << std::endl << std::endl;
      static double one[2]={1,0};
      static double minusone[2]={-1,0};
      cholmod_sparse *tmp =
          cholmod_add(P_, ptp, one, minusone, true, false, c_);
      double enn[2]={0.5*N_,0};
      static double ptfive[2]={-0.5,0};
      cholmod_sparse *R = cholmod_add(tmp, pwp, enn, ptfive, true, false, c_);
      cholmod_free_sparse(&ptp, c_);
      cholmod_free_sparse(&pwp, c_);
      cholmod_free_sparse(&tmp, c_);
      return R;
  }

  void MultivariateFNormalSufficientSparse::set_W(const SparseMatrix<double>& W)
{
    if (W_) cholmod_free_sparse(&W_, c_);
    cholmod_sparse Wtmp = Eigen::viewAsCholmod(
            W.selfadjointView<Eigen::Upper>());
    //W_ = cholmod_copy_sparse(&Wtmp, c_);
    W_ = cholmod_copy(&Wtmp, 0, 1, c_); //unsym for spsolve
}

  void MultivariateFNormalSufficientSparse::set_FX(const MatrixXd& FX,
          double cutoff)
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
        IMP_LOG_TERSE( "MVNsparse:   set FX to new matrix"<< std::endl);
        compute_sufficient_statistics(cutoff);
    }
  }

  void MultivariateFNormalSufficientSparse::set_JF(double f)
  {
    JF_=f;
    lJF_=-log(JF_);
    IMP_LOG_TERSE( "MVNsparse:   set JF = " << JF_ << " lJF_ = " << lJF_
            <<std::endl);
  }

  void MultivariateFNormalSufficientSparse::set_FM(const VectorXd& FM)
  {
    if (FM.rows() != FM_.rows() || FM.cols() != FM_.cols() || FM != FM_){
        if (FM.rows() != M_) {
            IMP_THROW("size mismatch for FM: got "
                    <<FM.rows() << " instead of " << M_, ModelException);
            }
        FM_=FM;
        IMP_LOG_TERSE( "MVNsparse:   set FM to new vector" << std::endl);
        compute_epsilon();
    }
  }

  void MultivariateFNormalSufficientSparse::set_Fbar(const VectorXd& Fbar)
  {
    if (Fbar.rows() != Fbar_.rows() || Fbar.cols() != Fbar_.cols()
            || Fbar != Fbar_){
        if (Fbar.rows() != M_) {
            IMP_THROW("size mismatch for Fbar: got "
                    << Fbar.rows() << " instead of " << M_, ModelException);
            }
        Fbar_=Fbar;
        IMP_LOG_TERSE( "MVNsparse:   set Fbar to new vector" << std::endl);
        compute_epsilon();
    }
  }

  void MultivariateFNormalSufficientSparse::set_Sigma(
          const SparseMatrix<double>& Sigma)
  {
        if (Sigma.cols() != Sigma.rows()) {
            IMP_THROW("need a square matrix!", ModelException);
            }
        //std::cout << "set_sigma" << std::endl;
        if (Sigma_) cholmod_free_sparse(&Sigma_, c_);
        cholmod_sparse A(Eigen::viewAsCholmod(
                            Sigma.selfadjointView<Eigen::Upper>()));
        Sigma_=cholmod_copy_sparse(&A, c_);
        //cholmod_print_sparse(Sigma_,"Sigma",c_);
        IMP_LOG_TERSE( "MVNsparse:   set Sigma to new matrix" << std::endl);
        IMP_LOG_TERSE( "MVNsparse:   computing Cholesky decomposition"
                << std::endl);
        // compute Cholesky decomposition for determinant and inverse
        //c_->final_asis=1; // setup LDLT calculation
        //c_->supernodal = CHOLMOD_SIMPLICIAL;
        // convert matrix to cholmod format
        //symbolic and numeric factorization
        L_ = cholmod_analyze(Sigma_, c_);
        int success = cholmod_factorize(Sigma_, L_, c_);
        //cholmod_print_factor(L_,"L",c_);

        if (success == 0 || L_->minor < L_->n)
            IMP_THROW("Sigma matrix is not positive semidefinite!",
                    ModelException);
        // determinant and derived constants
        cholmod_factor *Lcp(cholmod_copy_factor(L_, c_));
        cholmod_sparse *Lsp(cholmod_factor_to_sparse(Lcp,c_));
        double logDetSigma=0;
        if ((Lsp->itype != CHOLMOD_INT) &&
                (Lsp->xtype != CHOLMOD_REAL))
            IMP_THROW("types are not int and real, update them here first",
                    ModelException);
        int *p=(int*) Lsp->p;
        double *x=(double*) Lsp->x;
        for (size_t i=0; i < (size_t) M_; ++i)
            logDetSigma += std::log(x[p[i]]);
        cholmod_free_sparse(&Lsp,c_);
        cholmod_free_factor(&Lcp,c_);
        IMP_LOG_TERSE( "MVNsparse:   log det(Sigma) = "
                << logDetSigma << std::endl);
        IMP_LOG_TERSE( "MVNsparse:   det(Sigma) = "
                << exp(logDetSigma) << std::endl);
        norm_= std::pow(2*IMP::PI, -double(N_*M_)/2.0)
                    * exp(-double(N_)/2.0*logDetSigma);
        lnorm_=double(N_*M_)/2 * log(2*IMP::PI) + double(N_)/2 * logDetSigma;
        IMP_LOG_TERSE( "MVNsparse:   norm = " << norm_ << " lnorm = "
                << lnorm_ << std::endl);
        //inverse
        IMP_LOG_TERSE( "MVNsparse:   solving for inverse" << std::endl);
        cholmod_sparse* id = cholmod_speye(M_,M_,CHOLMOD_REAL,c_);
        if (P_) cholmod_free_sparse(&P_, c_);
        P_ = cholmod_spsolve(CHOLMOD_A, L_, id, c_);
        cholmod_free_sparse(&id, c_);
        if (!P_) IMP_THROW("Unable to solve for inverse!", ModelException);
        //WP
        IMP_LOG_TERSE( "MVNsparse:   solving for PW" << std::endl);
        if (PW_) cholmod_free_sparse(&PW_, c_);
        PW_ = cholmod_spsolve(CHOLMOD_A, L_, W_, c_);
        if (!PW_) IMP_THROW("Unable to solve for PW!", ModelException);
        IMP_LOG_TERSE( "MVNsparse:   done" << std::endl);
  }

  double MultivariateFNormalSufficientSparse::trace_WP() const
  {
      //solve for Sigma.X=W
      //cholmod_print_sparse(PW_,"PW",c_);
      //isolate diagonal
      cholmod_sparse *tmp = cholmod_band(PW_, 0, 0, 1, c_);
      //cholmod_print_sparse(tmp,"diag(PW)",c_);
      double trace=0;
      if ((tmp->itype != CHOLMOD_INT) || (tmp->xtype != CHOLMOD_REAL))
          IMP_THROW("matrix types different from int and double",
                  ModelException);
      double *x = (double *) tmp->x;
      for (size_t i=0; i < tmp->nzmax; ++i) trace += x[i];
      cholmod_free_sparse(&tmp, c_);
      IMP_LOG_TERSE( "MVNsparse:   trace(WP) = " << trace << std::endl);
      return trace;
  }

  // compute trans(epsilon)*Sigma^{-1}*epsilon by solving for the
  // rhs product and multiplying by trans(epsilon)
  // could be made more stable by using a LLt factorization and
  // solving for LPepsilon and then multiplying by
  // its transpose.
  double MultivariateFNormalSufficientSparse::mean_dist() const
{
    cholmod_dense *tmp = cholmod_solve(CHOLMOD_A, L_, epsilon_, c_);
    if (tmp->xtype != CHOLMOD_REAL)
          IMP_THROW("matrix type is not real, update code first",
                  ModelException);
    double dist=0;
    double *x1 = (double *) tmp->x;
    double *x2 = (double *) epsilon_->x;
    for (size_t i=0; i<tmp->nzmax; ++i) dist += x1[i]*x2[i];
    cholmod_free_dense(&tmp, c_);
    IMP_LOG_TERSE( "MVNsparse:   mean_dist = " << dist << std::endl);
    return dist;
}

  cholmod_sparse *MultivariateFNormalSufficientSparse::compute_PTP() const
{
  IMP_LOG_TERSE( "MVNsparse:   computing PTP" << std::endl);
  cholmod_sparse *eps = cholmod_dense_to_sparse(epsilon_, true, c_);
  cholmod_sparse *tmp = cholmod_spsolve(CHOLMOD_A, L_, eps, c_);
  cholmod_sparse *ptp = cholmod_aat(tmp, nullptr, 0, 1, c_);
  cholmod_free_sparse(&eps, c_);
  cholmod_free_sparse(&tmp, c_);
  return ptp;
}

  cholmod_sparse *MultivariateFNormalSufficientSparse::compute_PWP() const
{
      // PWP = PW
      IMP_LOG_TERSE( "MVNsparse:   computing PWP" << std::endl);
      //solve for X in Sigma*X=W
      cholmod_sparse *tmp = cholmod_spsolve(CHOLMOD_A, L_, W_, c_);
      //and then Y in trans(X)=Sigma*Y
      cholmod_sparse *tx = cholmod_transpose(tmp, 1, c_);
      cholmod_free_sparse(&tmp, c_);
      cholmod_sparse *R = cholmod_spsolve(CHOLMOD_A, L_, tx, c_);
      cholmod_free_sparse(&tx, c_);
      IMP_LOG_TERSE( "MVNsparse:   done" << std::endl);
      return R;
}

  void MultivariateFNormalSufficientSparse::compute_sufficient_statistics(
          double cutoff)
{
    IMP_LOG_TERSE( "MVNsparse:   computing sufficient statistics" << std::endl);
    Fbar_ = FX_.colwise().mean();
    compute_epsilon();
    //
    MatrixXd A(N_,M_);
    MatrixXd W(M_,M_);
    A = FX_.rowwise() - Fbar_.transpose();
    W = A.transpose()*A;
    SparseMatrix<double> Wsp(M_,M_);
    for (int j=0; j<M_; ++j)
    {
        Wsp.startVec(j);
        for (int i=0; i <= j; ++i)
        {
            if (std::abs(W(i,j)) > cutoff)
            {
                Wsp.insertBack(i,j) = W(i,j);
            }
        }
    }
    Wsp.finalize();
    cholmod_sparse Wtmp = Eigen::viewAsCholmod(
            Wsp.selfadjointView<Eigen::Upper>());
    if (Wtmp.x == nullptr)
    {
        W_ = cholmod_spzeros(M_,M_,0,CHOLMOD_REAL, c_);
    } else {
        //W_ = cholmod_copy_sparse(&Wtmp, c_);
        W_ = cholmod_copy(&Wtmp, 0, 1, c_); // don't store symmetic form
    }
    //std::cout << " A " << A << std::endl << std::endl;
    //std::cout << " W " << W << std::endl << std::endl;
    //std::cout << " W_ " << W_ << std::endl << std::endl;
    IMP_LOG_TERSE( "MVNsparse:   done sufficient statistics" << std::endl);
}

void MultivariateFNormalSufficientSparse::compute_epsilon()
{
    IMP_LOG_TERSE( "MVNsparse:      computing epsilon" << std::endl);
    VectorXd epsilon = Fbar_ - FM_;
    if (epsilon_) cholmod_free_dense(&epsilon_, c_);
    cholmod_dense epstmp = Eigen::viewAsCholmod(epsilon);
    epsilon_ = cholmod_copy_dense(&epstmp, c_);
    IMP_LOG_TERSE( "MVNsparse:      done epsilon" << std::endl);
}

IMPISD_END_NAMESPACE

#endif /* IMP_ISD_USE_CHOLMOD */
