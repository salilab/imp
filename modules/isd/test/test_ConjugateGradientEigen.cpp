/** testing of the Eigen CG. tests return true when they succeed.
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/internal/cg_eigen.h>
#include <IMP/macros.h>
#include <IMP/Pointer.h>
#include <IMP/Model.h>
#include <IMP/constants.h>
#include <IMP/base/exception.h>
#include <IMP/random.h>
#include <boost/random/uniform_real.hpp>
#include <Eigen/Dense>

namespace {

using namespace IMP::isd;

using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::VectorXi;

//boost::random::mt19937 rnsg;
boost::uniform_real<> uniform(0,1);
//boost::random::variate_generator<boost::mt19937&, boost::uniform_real<> >
//      rand(rnsg,uniform);

#define rand() uniform(IMP::random_number_generator)

#define FAIL(str) {std::cout << str << std::endl; return false;}

#define PRINT(str) std::cout << str << std::endl;

//not are equal, in tolerance.
bool naeq(double a, double b, double delta=1e-7){
    if (fabs(b)<=delta){
        if (fabs(a-b) > delta) {
            std::cout << a << " != " << b << " crit a-b: "
                << fabs(a-b) << " < " << delta << std::endl;
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

MatrixXd generate_factor(unsigned M, double lowest_eigenvalue,
        double condition)
{
    MatrixXd U(Eigen::MatrixXd::Random(M,M));
    U.triangularView<Eigen::Lower>().setZero();
    VectorXd diag(M);
    diag(0) = std::abs(lowest_eigenvalue);
    double incr = lowest_eigenvalue*(condition-1)/M;
    for (unsigned i=1; i<M-1; i++) diag(i) = diag(i-1)+rand()*incr;
    diag(M-1)=std::abs(lowest_eigenvalue)*condition;
    U.diagonal() = diag;
    //if (M<10) std::cout << U << std::endl;
    //std::cout << "M=" << M << " ev=" << lowest_eigenvalue
    //    << " matrix condition number " << condition << std::endl;
    return U;
}

bool test_cg(){
    //draw size of the matrix
    int M=1+floor(rand()*100);
    // draw number of cols of the rhs
    int N=1+floor(rand()*100);
    double condition = rand()*100;
    MatrixXd U(generate_factor(M, rand(), condition));
    //compute spd matrix
    //MatrixXd A;
    //A.selfadjointView<Eigen::Upper>() = U.transpose()*U;
    MatrixXd A(U.transpose()*U);
    //compute inverse
    Eigen::LLT<MatrixXd, Eigen::Upper> llt(A);
    MatrixXd P(llt.solve(MatrixXd::Identity(M,M)));
    //compute random rhs and solution to AX=B
    MatrixXd B(MatrixXd::Random(M,N));
    MatrixXd X(llt.solve(B));

    //create cg instance
    IMP_NEW(IMP::isd::internal::ConjugateGradientEigen, cg, ());
    cg->set_was_used(true);

    {
        //perturb A
        MatrixXd Upert(0.01*generate_factor(M, rand(), 2.0));
        //Apert.selfadjointView<Eigen::Upper>() = A + Upert.transpose()*Upert;
        MatrixXd Apert(A + Upert.transpose()*Upert);
        Eigen::LLT<MatrixXd, Eigen::Upper> lltpert(Apert);
        MatrixXd expected(lltpert.solve(B));
        cg->set_A(Apert);
        cg->set_B(B);
        cg->set_X0(X);
        cg->set_tol(1e-7);
        MatrixXd observed(cg->optimize(P, M));
        double resid=(observed-expected).norm()/expected.norm();
        if (resid > 1e-4)
        {
            FAIL("Perturb A failed with\tM=" << M <<
                    "\treturn code " << cg->info() <<
                    "\t(MxM = " << M*M
                    <<" )\tcondition number="<<condition
                    << "\tand residuals = " << resid );
        } else {
            std::cout<<"Perturb A succeeded with\tM=" << M <<
                    "\treturn code " << cg->info() <<
                    "\t(MxM = " << M*M
                    <<" )\tcondition number="<<condition
                    << "\tand residuals = " << resid << std::endl;
        }
    }

    {
        //perturb B
        MatrixXd Bpert(B + 0.1*MatrixXd::Random(M,N));
        MatrixXd expected(llt.solve(Bpert));
        cg->set_A(A);
        cg->set_B(Bpert);
        cg->set_X0(X);
        cg->set_tol(1e-7);
        MatrixXd observed(cg->optimize(P, M));
        double resid=(observed-expected).norm()/expected.norm();
        if (resid > 1e-4)
        {
            FAIL("Perturb B failed with\tM=" << M <<
                    "\treturn code " << cg->info() <<
                    "\t(MxM = " << M*M
                    <<" )\tcondition number="<<condition
                    << "\tand residuals = " << resid );
        } else {
            std::cout << "Perturb B succeeded with\tM=" << M <<
                    "\treturn code " << cg->info()  <<
                    "\t(MxM = " << M*M
                    <<" )\tcondition number="<<condition
                    << "\tand residuals = " << resid << std::endl;
        }
    }


    return true;
}
}

int main(int, char *[]) {
  try {
    PRINT("testing cg");
    int failures = 0;
    for (int i=0; i<30; i++){
      if (!test_cg()) {
        failures++;
      }
    }
    if (failures > 7) {
      std::cout << failures << " failures: failed" << std::endl;
      return 1;
    } else {
      std::cout << failures << " failures: passed" << std::endl;
      return 0;
    }
  } catch (const IMP::base::ExceptionBase &e) {
    std::cout << e.what() << std::endl;
  }
}
