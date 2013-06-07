/** testing of the Eigen CG. tests return true when they succeed.
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_INTERNAL_CG_EIGEN_H
#define IMPISD_INTERNAL_CG_EIGEN_H

#include <IMP/isd/isd_config.h>
#include <IMP/macros.h>
#include <IMP/Model.h>
#include <Eigen/Dense>

IMPISD_BEGIN_INTERNAL_NAMESPACE

using Eigen::MatrixXd;
using Eigen::VectorXd;


/** Simple implementation of the conjugate gradient method for matrices
 * This version is for Eigen dense matrices
 * */
class ConjugateGradientEigen : public base::Object {

private:

    MatrixXd A_,B_,X0_,R_;
    bool has_A_,has_B_,has_X0_,has_tol_,success_,col_success_;
    double tol_;
    unsigned M_,N_,nsteps_,vec_steps_;



public:

    //! Conjugate gradient algorithm for Eigen dense matrices
    ConjugateGradientEigen() : Object("cgEigen"),
                            has_A_(false), has_B_(false), has_X0_(false),
                            has_tol_(false), success_(false)
    {}

    /* \param[in] A the spd matrix for which the system AX=B
     *               is to be solved.
     */
    void set_A(const MatrixXd& A)
    {
        unsigned M = A.rows();
        IMP_USAGE_CHECK(A.cols() == M, "must provide spd matrix!");
        M_ = M;
        A_ = A;
        has_A_ = true;
        success_=false;
    }

    // \param[in] B
    void set_B(const MatrixXd& B)
    {
        N_ = B.cols();
        B_ = B;
        has_B_ = true;
        success_=false;
    }

    // \param[in] X0 an initial guess for X
    void set_X0(const MatrixXd& X0)
    {
        X0_ = X0;
        has_X0_ = true;
        success_=false;
    }

    // set tolerance
    void set_tol(double tol)
    {
        if (tol < tol_) success_ = false;
        IMP_USAGE_CHECK(tol> 0, "Must provide positive tolerance!");
        tol_=tol;
        has_tol_ = true;
    }

    // returns a number and a sign. The number is the total number of CG steps
    // performed. The sign is positive if all columns had residuals smaller than
    // the target, and negative otherwise.
    int info()
    {
        if(success_)
        {
            return nsteps_;
        } else {
            if (nsteps_<M_)
                std::cout << "======BUG========== nsteps="<<nsteps_
                    <<" < M="<<M_<<"  and failed!" << std::endl;
            return -nsteps_;
        }
    }

    //! CG optimization
    /*
     * \param[in] precond a preconditionner matrix.
     * \param[in] the maximum number of steps which will be performed
     * returns: the X matrix.
     */
    MatrixXd optimize(const MatrixXd& precond,
            unsigned max_steps_per_column)
    {
        success_=false;
        col_success_=true;
        nsteps_=0;
        IMP_USAGE_CHECK(X0_.rows() == M_ && X0_.cols() == N_,
                "X0 must have " << M_ <<" rows and " << N_ << " columns!");
        IMP_USAGE_CHECK(has_A_&&has_B_&&has_X0_&&has_tol_,
                "You must provide the matrices first!");
        MatrixXd X(X0_);
        for (unsigned col=0; col<N_; col++)
        {
            X.col(col).noalias() = optimize_vec(precond, B_.col(col),
                    X.col(col), max_steps_per_column);
            nsteps_ += vec_steps_;
        }
        if (col_success_) success_ = true;
        return X;
    }

private:

    VectorXd optimize_vec(const MatrixXd& precond,
            const VectorXd& b, const VectorXd& x0, unsigned max_steps)
    {
        //initial conditions
        VectorXd x(x0),r(b-A_*x0);
        VectorXd z;
        z.noalias() = precond*r;
        VectorXd p(z);
        VectorXd rold,zold;
        double a,beta,rz(r.transpose()*z),rzold,norm;
        //follow the gradients
        unsigned k;
        for (k=0; k<max_steps; k++)
        {
            VectorXd tmp(A_*p);
            a=rz/(p.transpose()*tmp);
            x += a*p;
            //rold=r;
            r -= a*tmp;
            norm=r.norm();
            if (norm < tol_)
            {
                col_success_ = col_success_ && true;
                break;
            }
            z.noalias() = precond*r;
            rzold=rz;
            rz=r.transpose()*z;
            beta = rz/rzold;
            p = z + beta*p;
            //std::cout << "vector_step "<<k
            //    <<" residual="<<norm
            //    <<" z(k+1)r(k)="<<z.transpose()*rold
            //    <<std::endl;
        }
        vec_steps_ = k;
        if (k == max_steps) col_success_ = false;
        //std::cout << "optimize_vector success=" << col_success_
        //    << " norm=" << norm << " max_steps=" << max_steps
        //    << " k="<<k << std::endl;
        return x;
    }


    IMP_OBJECT_METHODS(ConjugateGradientEigen);

};

IMPISD_END_INTERNAL_NAMESPACE


#endif /* IMPISD_INTERNAL_CG_EIGEN_H */
