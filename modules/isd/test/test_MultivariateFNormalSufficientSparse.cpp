/**
 *  \file test_MultivariateFNormalSufficientSparse.cpp
 *  \brief testing of the Multivariate FNormal.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/base/exception.h>
#include <math.h>
#include <IMP/base/random.h>
#include <boost/random/uniform_real.hpp>
#include <IMP/macros.h>
#include <IMP/base/Pointer.h>
#include <IMP/isd/MultivariateFNormalSufficient.h>
#include <IMP/isd/FNormal.h>

#ifdef IMP_ISD_USE_CHOLMOD

#include <IMP/isd/MultivariateFNormalSufficientSparse.h>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unsupported/Eigen/CholmodSupport>
#include <ufsparse/cholmod.h>

using namespace IMP::isd;

using Eigen::SparseMatrix;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::VectorXi;

//boost::random::mt19937 rnsg;
boost::uniform_real<> uniform(0,1);
//boost::random::variate_generator<boost::mt19937&, boost::uniform_real<> >
//rand(rnsg,uniform);

cholmod_common c_;

#define rand() uniform(IMP::base::random_number_generator)

#define FAIL(str) {std::cout << str << std::endl; return false;}

#define PRINT(str) std::cout << str << std::endl;

#define RUNTEST(name,nreps) \
for (int i=0; i<nreps; i++){\
    if (!name()){\
        std::cerr << " failed" << std::endl;\
        return 1;\
    }\
}\
std::cerr << " passed" << std::endl;

#define RUNTEST_N(name,nreps,N) \
for (int i=0; i<nreps; i++){\
    if (!name(N)){\
        std::cerr << " failed" << std::endl;\
        return 1;\
    }\
}\
std::cerr << " passed" << std::endl;

//not are equal, in tolerance.
bool naeq(double a, double b, double delta=1e-7){
    if (fabs(b)<=delta){
        if (fabs(a-b) > delta) {
            std::cout << a << " != " << b << " crit a-b: " << fabs(a-b)
                << " < " << delta << std::endl;
            return true;
        }
    } else {
        if (a*b <0 || fabs(fabs((double)a/b)-1) > delta) {
            std::cout << a << " != " << b << " crit a/b: "
                << fabs(fabs(a/b)-1) << " < " << delta << std::endl;
            return true;
        }
    }
    return false;
}

//test when M=100 and N=10
bool test_sanity(){
    //observation matrix
    int M=1+floor(rand()*100);
    int N=1+floor(rand()*100);
    MatrixXd FA(N,M);
    for (int i=0; i<N; i++){
        for (int j=0; j<M; j++){
            FA(i,j) = (rand()*100-50);
        }
    }
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(M);
    for (int i=0; i<M; i++){
        FM(i)=rand()-50;
    }
    //Precision matrix
    SparseMatrix<double> Sigma(M,M);
    for (int i=0; i<M; i++){
        Sigma.startVec(i);
        Sigma.insertBack(i,i)=1+rand()*10;
    }
    Sigma.finalize();

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv,
            (FA,JA,FM,Sigma,&c_));
    //mv->set_was_used(true);

    {
    //evaluate
    double observed=exp(-mv->evaluate());
    double expected=mv->density();
    if (naeq(observed,expected)) FAIL("density->evaluate");
    }

    {
    //density
    double observed=-log(mv->density());
    double expected=mv->evaluate();
    if (! (std::numeric_limits<double>::infinity() == observed)){
        if (naeq(observed,expected)) FAIL("evaluate->density");
    }
    }
    return true;
}

//the distribution should coincide with the normal distribution
//when N=M=1
bool test_degenerate(){
    //observation matrix
    MatrixXd FA(1,1);
    FA(0,0)=rand()*10;
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(1);
    FM(0)=rand()*10;
    //Precision matrix
    SparseMatrix<double> Sigma(1,1);
    Sigma.startVec(0);
    Sigma.insertBack(0,0)=1.0+rand();
    Sigma.finalize();

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv,
            (FA,JA,FM,Sigma,&c_));
    //mv->set_was_used(true);
    IMP_NEW(IMP::isd::FNormal, fn,
            (FA(0,0),JA,FM(0),sqrt(MatrixXd(Sigma)(0,0))));
    //fn->set_was_used(true);

    {
    //evaluate
    double observed=mv->evaluate();
    double expected=fn->evaluate();
    if (naeq(observed,expected)) FAIL("evaluate");
    }

    {
    //density
    double observed=mv->density();
    double expected=fn->density();
    if (naeq(observed,expected)) FAIL("density");
    }

    {
    //evaluate_derivative_FM
    cholmod_dense *obs=mv->evaluate_derivative_FM();
    double observed = ((double*)obs->x)[0];
    cholmod_free_dense(&obs, &c_);
    double expected=fn->evaluate_derivative_FM();
    if (naeq(observed,expected)) FAIL("evaluate_derivative_FM");
    }

    {
    //evaluate_derivative_Sigma
    cholmod_sparse *obs=mv->evaluate_derivative_Sigma();
    double observed = ((double*)obs->x)[0];
    cholmod_free_sparse(&obs, &c_);
    double expected=fn->evaluate_derivative_sigma();
    expected=expected/(2*sqrt(MatrixXd(Sigma)(0,0)));
    if (naeq(observed,expected)) FAIL("evaluate_derivative_Sigma");
    }

    return true;
}

//the distribution should coincide with two normal distributions
//when N=2 and M=1
bool test_degenerate_N2M1(){
    //observation matrix
    MatrixXd FA(2,1);
    FA(0,0) = rand()*10;
    FA(1,0) = rand()*10;
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(1);
    FM(0)=rand()*10;
    //Covariance matrix
    SparseMatrix<double> Sigma(1,1);
    Sigma.startVec(0);
    Sigma.insertBack(0,0)=1.0+rand();
    Sigma.finalize();

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv,
            (FA,JA,FM,Sigma,&c_));
    //double mean =(FA(0,0)+FA(1,0))/2.0;
    //double ss = (IMP::square(FA(0,0) - mean) + IMP::square(FA(1,0) - mean));
    //PRINT(" epsilon " << mean-FM(0) << " ss " << ss);
    //mv->set_was_used(true);
    IMP_NEW(IMP::isd::FNormal, fn,
            (FA(0,0),JA,FM(0),sqrt(MatrixXd(Sigma)(0,0))));
    IMP_NEW(IMP::isd::FNormal, fn2,
            (FA(1,0),JA,FM(0),sqrt(MatrixXd(Sigma)(0,0))));
    //fn->set_was_used(true);

    {
    //evaluate
    double observed=mv->evaluate();
    double expected=fn->evaluate()+fn2->evaluate();
    //double mean =(FA(0,0)+FA(1,0))/2.0;
    //double meandist = 1.0*IMP::square(mean-FM(0))/MatrixXd(Sigma)(0,0);
    //double ss = 0.5*(IMP::square(FA(0,0) - mean)
    //  + IMP::square(FA(1,0) - mean))/MatrixXd(Sigma)(0,0);
    //double norm = std::log(2*IMP::PI*IMP::square(MatrixXd(Sigma)(0,0)));
    //double recalc = ss+meandist+norm;
    //PRINT(" eval obs " << observed << " exp " << expected
    //        << " md " << meandist << " ss " << ss
    //        << " sigma " << MatrixXd(Sigma)(0,0)
    //        << " norm " << norm << " recalc " << recalc );
    if (naeq(observed,expected)) FAIL("evaluate");
    }
    {
    //density
    double observed=mv->density();
    double expected=fn->density()*fn2->density();
    if (naeq(observed,expected)) FAIL("evaluate");
    }

    {
    //evaluate_derivative_FM
    cholmod_dense *obs=mv->evaluate_derivative_FM();
    double observed=((double*)obs->x)[0];
    cholmod_free_dense(&obs,&c_);
    double expected=fn->evaluate_derivative_FM()+fn2->evaluate_derivative_FM();
    if (naeq(observed,expected)) FAIL("evaluate_derivative_FM");
    }

    {
    //evaluate_derivative_Sigma
    cholmod_sparse *obs=mv->evaluate_derivative_Sigma();
    double observed = ((double*)obs->x)[0];
    cholmod_free_sparse(&obs,&c_);
    double expected=
        fn->evaluate_derivative_sigma()/(2*sqrt(MatrixXd(Sigma)(0,0)))
        + fn2->evaluate_derivative_sigma()/(2*sqrt(MatrixXd(Sigma)(0,0)));
    if (naeq(observed,expected)) FAIL("evaluate_derivative_Sigma");
    }
    return true;
}

//the distribution should coincide with two normal distributions
//when N=1 and M=2 in the absence of correlations
bool test_degenerate_N1M2(){
    //observation matrix
    MatrixXd FA(1,2);
    FA(0,0) = rand()*10;
    FA(0,1) = rand()*10;
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(2);
    FM(0)=rand()*10;
    FM(1)=rand()*10;
    //Covariance matrix
    SparseMatrix<double> Sigma(2,2);
    Sigma.startVec(0);
    Sigma.insertBack(0,0)=1.0+rand();
    Sigma.startVec(1);
    Sigma.insertBack(1,1)=1.0+rand();
    Sigma.finalize();

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv,
            (FA,JA,FM,Sigma,&c_));
    //double mean =(FA(0,0)+FA(1,0))/2.0;
    //double ss = (IMP::square(FA(0,0) - mean) + IMP::square(FA(1,0) - mean));
    //PRINT(" epsilon " << mean-FM(0) << " ss " << ss);
    //mv->set_was_used(true);
    IMP_NEW(IMP::isd::FNormal, fn,
            (FA(0,0),JA,FM(0),sqrt(MatrixXd(Sigma)(0,0))));
    IMP_NEW(IMP::isd::FNormal, fn2,
            (FA(0,1),JA,FM(1),sqrt(MatrixXd(Sigma)(1,1))));
    //fn->set_was_used(true);

    {
    //evaluate
    double observed=mv->evaluate();
    double expected=fn->evaluate()+fn2->evaluate();
    //double mean =(FA(0,0)+FA(1,0))/2.0;
    //double meandist = 2.0*IMP::square(mean-FM(0))/Sigma(0,0);
    //double ss = (IMP::square(FA(0,0) - mean)
    //   + IMP::square(FA(1,0) - mean))/Sigma(0,0);
    //PRINT(" eval " << observed << " "
    //  << expected << " " << meandist << " " << ss);
    if (naeq(observed,expected)) FAIL("evaluate");
    }
    {
    //density
    double observed=mv->density();
    double expected=fn->density()*fn2->density();
    if (naeq(observed,expected)) FAIL("evaluate");
    }

    {
    //evaluate_derivative_FM
    cholmod_dense *obs =mv->evaluate_derivative_FM();
    double *observed = (double *) obs->x;
    double expected=fn->evaluate_derivative_FM();
    if (naeq(observed[0],expected)) FAIL("evaluate_derivative_FM 1");
    expected=fn2->evaluate_derivative_FM();
    if (naeq(observed[1],expected)) FAIL("evaluate_derivative_FM 2");
    cholmod_free_dense(&obs,&c_);
    }

    {
    //evaluate_derivative_Sigma
    cholmod_sparse *tmp=mv->evaluate_derivative_Sigma();
    cholmod_dense *obs = cholmod_sparse_to_dense(tmp, &c_);
    double *observed = (double*)obs->x;
    double expected
        =fn->evaluate_derivative_sigma()/(2*sqrt(MatrixXd(Sigma)(0,0)));
    if (naeq(observed[0],expected))
        FAIL("evaluate_derivative_Sigma 1 1");
    expected=fn2->evaluate_derivative_sigma()/(2*sqrt(MatrixXd(Sigma)(1,1)));
    if (naeq(observed[3],expected))
        FAIL("evaluate_derivative_Sigma 2 2");
    cholmod_free_dense(&obs, &c_);
    cholmod_free_sparse(&tmp, &c_);
    }
    return true;
}

//test when M=1 and N>=1
bool test_1D(int N){
    //observation matrix: N values between 0 and 10
    MatrixXd FA(N,1);
    for (int i=0; i<N; i++){
        FA(i,0)=rand()*10;
    }
    //Jacobian: normal distribution.
    double JA =1.0;
    //mean vector
    VectorXd FM(1);
    FM(0) = rand()*10;
    //Covariance matrix
    SparseMatrix<double> Sigma(1,1);
    Sigma.startVec(0);
    Sigma.insertBack(0,0)=1.0+rand();
    Sigma.finalize();

    IMP_NEW(IMP::isd::FNormal, fn,
            (FA(0,0),JA,FM(0),sqrt(MatrixXd(Sigma)(0,0))));
    //IMP_NEW(IMP::isd::FNormal, fn2, (FA(0,1),JA,FM(1),sqrt(Sigma(0,0))));
    //fn->set_was_used(true);
    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv,
            (FA,JA,FM,Sigma,&c_));
    //mv->set_was_used(true);

    //compute sufficient statistics
    double sample_mean = 0;
    for (int i=0; i<N; i++){
        sample_mean += FA(i,0)/double(N);
    }
    double sum_squares = 0;
    for (int i=0; i<N; i++){
        sum_squares += IMP::square(FA(i,0) - sample_mean);
    }

    {
    //test evaluate()
    double observed=mv->evaluate();
    double sigma=sqrt(MatrixXd(Sigma)(0,0));
    double expected=double(N)*log((sqrt(2*IMP::PI)*sigma)) +
        1/(2*sigma*sigma) * (double(N)*IMP::square(sample_mean - FM(0))
                                        + sum_squares);
    //PRINT("evaluate " << observed << " " << expected << " "
    //<< fn->evaluate() +  fn2->evaluate());
    if (naeq(observed,expected)) FAIL("evaluate");
    }

    {
    //test density()
    double observed = mv->density();
    double expected=pow(2*IMP::PI*MatrixXd(Sigma)(0,0),-N/2.0) * exp(
            -0.5/MatrixXd(Sigma)(0,0)
                *( (double)N*IMP::square(sample_mean - FM(0)) + sum_squares ));
    //PRINT("density " << observed << " " << expected << " " << fn->density());
    if (naeq(observed,expected)) FAIL("density");
    }

    {
    //test evaluate_derivative_FM()
    cholmod_dense *obs = mv->evaluate_derivative_FM();
    double observed = ((double*)obs->x)[0];
    cholmod_free_dense(&obs,&c_);
    double expected = -double(N)/MatrixXd(Sigma)(0,0)*(sample_mean - FM(0));
    //PRINT("FM " << observed(0) << " " << expected );
    if (naeq(observed,expected)) FAIL("evaluate_derivative_FM");
    }

    {
    //test evaluate_derivative_Sigma()
    cholmod_sparse *obs = mv->evaluate_derivative_Sigma();
    double observed = ((double*)obs->x)[0];
    cholmod_free_sparse(&obs,&c_);
    double expected = (double)N/(2.0*MatrixXd(Sigma)(0,0))
        - 0.5 / IMP::square(MatrixXd(Sigma)(0,0))
            * ((double)N*IMP::square(sample_mean - FM(0)) + sum_squares);
    //PRINT("Sigma " << observed(0,0) << " " << expected
    //<< " " << fn->evaluate_derivative_sigma()/(2*sqrt(Sigma(0,0))));
    if (naeq(observed,expected)) FAIL("evaluate_derivative_Sigma");
    }

    //all tests succeeded.
    return true;
}

//test when M=2 and N=1
bool test_2D(){
    //observation matrix
    MatrixXd FA(1,2);
    FA(0,0)=0.5;
    FA(0,1)=1.0;
    //FA(1,0)=0.7;
    //FA(1,1)=1.2;
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(2);
    FM(0)=0.0;
    FM(1)=2.0;
    //Precision matrix
    SparseMatrix<double> Sigma(2,2);
    double sigma1=2.0;
    double sigma2=1.0;
    double rho=0.5;
    Sigma.startVec(0);
    Sigma.insertBack(0,0)=sigma1*sigma1;
    Sigma.insertBack(1,0)=rho*sigma1*sigma2;
    Sigma.startVec(1);
    Sigma.insertBack(0,1)=rho*sigma1*sigma2;
    Sigma.insertBack(1,1)=sigma2*sigma2;
    Sigma.finalize();
    //double factor=1/(rho*rho-1);
    //P(0,0)=-factor/(sigma1*sigma1);
    //P(1,1)=-factor/(sigma2*sigma2);
    //P(0,1)=factor*rho/(sigma1*sigma2);
    //P(1,0)=factor*rho/(sigma1*sigma2);

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv,
            (FA,JA,FM,Sigma,&c_));
    //mv->set_was_used(true);

    {
    //evaluate
    double observed=mv->evaluate();
    double expected=log(2*IMP::PI*sigma1*sigma2*sqrt(1-rho*rho)) +
        1/(2*(1-rho*rho)) * ( IMP::square(FA(0,0) - FM(0))/(sigma1*sigma1)
                + IMP::square(FA(0,1)-FM(1))/(sigma2*sigma2)
                - 2*rho*(FA(0,0)-FM(0))*(FA(0,1)-FM(1))/(sigma1*sigma2));
    if (naeq(observed,expected)) FAIL("evaluate");
    }

    {
    //density
    double observed=mv->density();
    double expected=1/(2*IMP::PI*sigma1*sigma2*sqrt(1-rho*rho))*exp(
        -1/(2*(1-rho*rho)) * ( IMP::square(FA(0,0) - FM(0))/(sigma1*sigma1)
            + IMP::square(FA(0,1)-FM(1))/(sigma2*sigma2)
                - 2*rho*(FA(0,0)-FM(0))*(FA(0,1)-FM(1))/(sigma1*sigma2)));
    if (naeq(observed,expected)) FAIL("density");
    }

    {
    //test_evaluate_derivative_FM
    cholmod_dense *obs=mv->evaluate_derivative_FM();
    double *observed = (double *)obs->x;
    double expected=((FM(1)-FA(0,1))*rho*sigma1
            + (FA(0,0)-FM(0))*sigma2)/((-1+rho*rho)*sigma1*sigma1*sigma2);
    if (naeq(observed[0],expected)) FAIL("derivative_FM(0)");
    expected=((FA(0,1)-FM(1))*sigma1
            + (FM(0)-FA(0,0))*rho*sigma2)/((-1+rho*rho)*sigma1*sigma2*sigma2);
    if (naeq(observed[1],expected)) FAIL("derivative_FM(1)");
    cholmod_free_dense(&obs, &c_);
    }

    {
    //test_evaluate_derivative_Sigma
    MatrixXd tmp(Sigma);
    double det = tmp(0,0)*tmp(1,1)-tmp(1,0)*tmp(1,0);
    double eps1=FA(0,0)-FM(0);
    double eps2=FA(0,1)-FM(1);
    cholmod_sparse *obs=mv->evaluate_derivative_Sigma();
    cholmod_dense *dense=cholmod_sparse_to_dense(obs, &c_);
    cholmod_free_sparse(&obs, &c_);
    double *observed = (double *)dense->x;
    double expected=0.5/IMP::square(det)
        * (   IMP::square(MatrixXd(Sigma)(1,1))
                * (MatrixXd(Sigma)(0,0) - IMP::square(eps1))
            - IMP::square(MatrixXd(Sigma)(1,0))
                * (MatrixXd(Sigma)(1,1) + IMP::square(eps2))
            + 2*MatrixXd(Sigma)(1,0)*MatrixXd(Sigma)(1,1)*eps1*eps2 );
    if (naeq(observed[0],expected)) FAIL("derivative_Sigma(0,0)");
    expected=0.5/IMP::square(det)
        * (   IMP::square(MatrixXd(Sigma)(0,0))
                * (MatrixXd(Sigma)(1,1) - IMP::square(eps2))
            - IMP::square(MatrixXd(Sigma)(1,0))
                * (MatrixXd(Sigma)(0,0) + IMP::square(eps1))
            + 2*MatrixXd(Sigma)(1,0)*MatrixXd(Sigma)(0,0)*eps2*eps1 );
    if (naeq(observed[3],expected)) FAIL("derivative_Sigma(1,1)");
    expected=0.5/IMP::square(det)
        * ( MatrixXd(Sigma)(1,0)*(-det+MatrixXd(Sigma)(1,1)*IMP::square(eps1)
                    +MatrixXd(Sigma)(0,0)*IMP::square(eps2))
            - eps1*eps2*(IMP::square(MatrixXd(Sigma)(1,0))
                +MatrixXd(Sigma)(0,0)*MatrixXd(Sigma)(1,1)) );
    if (naeq(observed[2],expected)) FAIL("derivative_Sigma(0,1)");
    if (naeq(observed[1],expected)) FAIL("derivative_Sigma(1,0)");
    cholmod_free_dense(&dense,&c_);
    }
    return true;
}

//test when M=100 and N=10
bool test_100D(){
    //observation matrix
    MatrixXd FA(10,100);
    for (int i=0; i<10; i++){
        for (int j=0; j<100; j++){
            // a little bit beneath the maximum.
            FA(i,j) = double( i-2 + j-54 );
        }
    }
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(100);
    for (int i=0; i<100; i++){
        FM(i)=i-49.0;
    }
    //Covariance matrix
    SparseMatrix<double> Sigma(100,100);
    for (int i=0; i<100; i++){
            Sigma.startVec(i);
            Sigma.insertBack(i,i)=1.0;
    }
    Sigma.finalize();

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv,
            (FA,JA,FM,Sigma,&c_));
    //mv->set_was_used(true);

    {
    //evaluate
    double observed=mv->evaluate();
    double expected=8168.938533204672717;
    if (naeq(observed,expected)) FAIL("evaluate");
    }

    return true;
}

//test that sparse and non-sparse versions agree
bool test_sparseness(){
    //observation matrix
    int M=100;
    int N=10;
    MatrixXd FA(N,M);
    for (int i=0; i<N; i++){
        for (int j=0; j<M; j++){
            FA(i,j) = rand();
        }
    }
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(M);
    for (int i=0; i<M; i++){
        FM(i)=rand();
    }
    //Covariance matrix should be sparse with a bandwidth of 6 at delta=1e-7
    SparseMatrix<double> Sigma(M,M);
    for (int j=0; j<M; j++){
            Sigma.startVec(j);
            for (int i=0; i<=j; i++) // fill upper corner
            {
                double val = std::exp(-0.5*std::pow(double(std::abs(i-j)),2));
                if (std::abs(val) > 1e-7)
                    Sigma.insertBack(i,j) = val;
            }
    }
    Sigma.finalize();

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mvsp,
            (FA,JA,FM,Sigma,&c_));
    IMP_NEW(IMP::isd::MultivariateFNormalSufficient, mv,
            (FA,JA,FM,MatrixXd(Sigma)));

    {
    //evaluate
    double observed=mvsp->evaluate();
    double expected=mv->evaluate();
    if (naeq(observed,expected,1e-5)) FAIL("evaluate");
    }

    {
    //derivative wrt mu
    cholmod_dense *obs=mvsp->evaluate_derivative_FM();
    double *observed = (double *)obs->x;
    VectorXd expected=mv->evaluate_derivative_FM();
    for (int i=0; i<M; i++)
    {
        if (naeq(observed[i],expected(i),1e-5))
            FAIL("derivative_FM("<<i<<")");
    }
    cholmod_free_dense(&obs,&c_);
    }

    {
    //derivative wrt sigma
    cholmod_sparse *obs=mvsp->evaluate_derivative_Sigma();
    cholmod_dense *tmp=cholmod_sparse_to_dense(obs, &c_);
    cholmod_free_sparse(&obs,&c_);
    double *observed = (double *)tmp->x;
    int d = tmp->d;
    MatrixXd expected=mv->evaluate_derivative_Sigma();
    for (int i=0; i<M; i++)
    {
        for (int j=0; j<M; j++)
        {
        if (naeq(observed[i+j*d],expected(i,j),1e-5))
            FAIL("derivative_Sigma("<<i<<","<<j<<")");
        }
    }
    cholmod_free_dense(&tmp, &c_);
    }

    return true;
}

/*
//test when M=2 and N=1 TODO: P->Sigma
bool gen_2D(){
    //observation matrix
    MatrixXd FA(1,2);
    FA(0,0)=0.5;
    FA(0,1)=1.0;
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(2);
    FM(0)=0.0;
    FM(1)=2.0;
    //Precision matrix
    MatrixXd P(2,2);
    double sigma1=2.0;
    double sigma2=1.0;
    double rho=0.5;
    //P(0,0)=sigma1*sigma1;
    //P(1,1)=sigma2*sigma2;
    //P(0,1)=rho*sigma1*sigma2;
    //P(1,0)=rho*sigma1*sigma2;
    double factor=1/(rho*rho-1);
    P(0,0)=-factor/(sigma1*sigma1);
    P(1,1)=-factor/(sigma2*sigma2);
    P(0,1)=factor*rho/(sigma1*sigma2);
    P(1,0)=factor*rho/(sigma1*sigma2);

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv, (FA,JA,FM,P));
    //mv->set_was_used(true);

    for (int i=0; i<1000; i++){
    double mu1=((double)i - 500)/100.0;
    FM(0)=mu1;
    mv->set_FM(FM);
    //evaluate
    VectorXd observed=mv->evaluate_derivative_FM();
    double expected=((FM(1)-FA(0,1))*rho*sigma1
      + (FA(0,0)-FM(0))*sigma2)/((-1+rho*rho)*sigma1*sigma1*sigma2);
    double expected2=((FA(0,1)-FM(1))*sigma1
      + (FM(0)-FA(0,0))*rho*sigma2)/((-1+rho*rho)*sigma1*sigma2*sigma2);
    PRINT(mu1 << " " << observed(0)
      << " " << expected << " " << observed(1) << " " << expected2);
    }
    return true;
}

//test with N MultivariateFNormalSufficientSparse instances TODO
bool test_degenerate_2(int N){
    //observation matrix
    MatrixXd FA(N,1);
    for (int i=0; i<N; i++){
        FA(i,0)=rand()*10;
    }
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(1,rand()*10);
    //Precision matrix
    MatrixXd P(1,1,1.0+rand());

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv, (FA,JA,FM,P));
    //mv->set_was_used(true);
    base::Pointer<IMP::isd::MultivariateFNormalSufficientSparse> * fn;
    fn = new base::Pointer<IMP::isd::MultivariateFNormalSufficientSparse> (N);
    for (int i=0; i< N; i++){
        MatrixXd a(1,1,FA(i,0));
        VectorXd m(1,FM(i));
        fn(i) = new IMP::isd::MultivariateFNormalSufficientSparse(a,JA,m,P);
        //fn(i)->set_was_used(true);
    }

    {
    //evaluate
    double observed=mv->evaluate();
    double expected=0.0;
    for (int i=0; i<N; i++){
        expected += fn(i)->evaluate();
    }
    if (naeq(observed,expected)) FAIL("evaluate");
    }

    {
    //density
    double observed=mv->density();
    double expected=1.0;
    for (int i=0; i<N; i++){
        expected *= fn(i)->density();
    }
    if (naeq(observed,expected)) FAIL("density");
    }

    {
    //evaluate_derivative_FM
    double observed=mv->evaluate_derivative_FM()(0);
    double expected=0.0;
    for (int i=0; i<N; i++){
        expected += fn(i)->evaluate_derivative_FM()(0);
    }
    if (naeq(observed,expected)) FAIL("evaluate_derivative_FM");
    }

    {
    //evaluate_derivative_P
    double observed=mv->evaluate_derivative_P()(0,0);
    double expected=0.0;
    for (int i=0; i<N; i++){
        expected += fn(i)->evaluate_derivative_P()(0,0);
    }
    if (naeq(observed,expected)) FAIL("evaluate_derivative_P");
    }

    return true;
}
//test with N FNormal instances
//test with N MultivariateFNormalSufficientSparse instances TODO
bool test_test(int N){
    //observation matrix
    MatrixXd FA(N,1,0.0);
    //for (int i=0; i<N; i++){
    //    FA(i,0)=rand()*10;
    //}
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(1,0.0);
    //Precision matrix
    MatrixXd P(1,1,1.1);

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv, (FA,JA,FM,P));
    //mv->set_was_used(true);
    base::Pointer<IMP::isd::MultivariateFNormalSufficientSparse> * fn;
    base::Pointer<IMP::isd::FNormal> * fn2;
    fn = new base::Pointer<IMP::isd::MultivariateFNormalSufficientSparse> (N);
    fn2 = new base::Pointer<IMP::isd::FNormal> (N);
    for (int i=0; i< N; i++){
        MatrixXd a(1,1,FA(i,0));
        VectorXd m(1,FM(i));
        fn(i) = new IMP::isd::MultivariateFNormalSufficientSparse(a,JA,m,P);
        //fn(i)->set_was_used(true);
        fn2(i) = new IMP::isd::FNormal(a(0,0),JA,m(0),1/sqrt(P(0,0)));
        //fn2(i)->set_was_used(true);
    }

    {
    //evaluate
    double observed=mv->evaluate();
    double dobs=mv->density();
    double expected=0.0;
    double expected2=0.0;
    double dexp=1.0;
    double dexp2=1.0;
    for (int i=0; i<N; i++){
        expected += fn(i)->evaluate();
        expected2 += fn2(i)->evaluate();
        dexp *= fn(i)->density();
        dexp2 *= fn2(i)->density();
    }
    PRINT(N << " "<< observed << " "
            << expected << " " << expected2
            << " " << dobs << " " << dexp << " " << dexp2);
    }

    return true;
}

//test with N FNormal instances TODO
bool test_degenerate_N(int N){
    //observation matrix
    MatrixXd FA(N,1);
    for (int i=0; i<N; i++){
        FA(i,0)=rand()*10;
    }
    //Jacobian
    double JA =1.0;
    //mean vector
    VectorXd FM(1);
    FM(0) = rand()*10;
    //Precision matrix
    MatrixXd P(1,1,1.0+rand());

    IMP_NEW(IMP::isd::MultivariateFNormalSufficientSparse, mv, (FA,JA,FM,P));
    //mv->set_was_used(true);
    base::Pointer<IMP::isd::FNormal> * fn;
    fn = new base::Pointer<IMP::isd::FNormal> (N);
    for (int i=0; i< N; i++){
        fn(i) = new IMP::isd::FNormal(FA(i,0),JA,FM(i),sqrt(1.0/P(0,0)));
        //fn(i)->set_was_used(true);
    }

    {
    //evaluate
    double observed=mv->evaluate();
    double expected=0.0;
    for (int i=0; i<N; i++){
        expected += fn(i)->evaluate();
    }
    if (naeq(observed,expected)) FAIL("evaluate");
    }

    {
    //density
    double observed=mv->density();
    double expected=1.0;
    for (int i=0; i<N; i++){
        expected *= fn(i)->density();
    }
    if (naeq(observed,expected)) FAIL("density");
    }

    {
    //evaluate_derivative_FM
    double observed=mv->evaluate_derivative_FM()(0);
    double expected=0.0;
    for (int i=0; i<N; i++){
        expected += fn(i)->evaluate_derivative_FM();
    }
    if (naeq(observed,expected)) FAIL("evaluate_derivative_FM");
    }

    {
    //evaluate_derivative_P
    double observed=mv->evaluate_derivative_P()(0,0);
    double expected=0.0;
    for (int i=0; i<N; i++){
        expected += fn(i)->evaluate_derivative_sigma()/(-2.0*pow(P(0,0),1.5));
    }
    if (naeq(observed,expected)) FAIL("evaluate_derivative_P");
    }

    return true;
}
*/

#endif /*IMP_ISD_USE_CHOLMOD*/
int main(int, char *[]) {
#ifdef IMP_ISD_USE_CHOLMOD
    cholmod_start(&c_);
    try {
    //test_mu(20);
    //return 0;
    //for (int i=1; i<100; i++){
    //    test_test(i);
    //}
    //return 0;
    //gen_2D();
    //return 0;
    //
    PRINT("sanity");
    RUNTEST(test_sanity,100);
    PRINT("degenerate N=1 M=1");
    RUNTEST(test_degenerate,100);
    PRINT("degenerate N=2 M=1");
    RUNTEST(test_degenerate_N2M1,100);
    PRINT("degenerate N=1 M=2");
    RUNTEST(test_degenerate_N1M2,100);
    PRINT("1D 1");
    RUNTEST_N(test_1D,100,1);
    PRINT("1D 2");
    RUNTEST_N(test_1D,100,2);
    PRINT("1D 10");
    RUNTEST_N(test_1D,100,10);
    PRINT("1D 100");
    RUNTEST_N(test_1D,100,100);
    //return 0;
    //PRINT("degenerate N=10");
    //RUNTEST_N(test_degenerate_N,100,10);
    //PRINT("degenerate N=100");
    //RUNTEST_N(test_degenerate_N,100,100);
    PRINT("2D");
    RUNTEST(test_2D,1);
    PRINT("100D");
    RUNTEST(test_100D,1);
    PRINT("sparseness");
    RUNTEST(test_sparseness,100);
    //TODO
    //PRINT("setting values");
    //RUNTEST(test_setval,1);
    return 0;
    } catch (const IMP::base::ExceptionBase& e) {
        std::cerr << e.what() << std::endl;
    }
    cholmod_finish(&c_);
#endif /*IMP_ISD_USE_CHOLMOD*/
#ifndef IMP_ISD_USE_CHOLMOD
    std::cout << "test skipped because CHOLMOD is not available" << std::endl;
    return 0;
#endif
}
