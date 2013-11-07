/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include "profile_variance.h"
#include <IMP.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

#include <iostream>
#include <fstream>
#include <Eigen/Dense>

namespace {
using namespace IMP;
using namespace IMP::base;
using namespace IMP::isd;
using Eigen::Array3d;
using Eigen::ArrayXd;
using Eigen::ArrayXXd;
using Eigen::MatrixXd;
using saxs::internal::SincFunction;
using saxs::internal::SincCosFunction;
typedef Eigen::Array<double,Eigen::Dynamic, 3> ArrayCoord;
}

int main(int argc, char **argv) {
    IMP::base::setup_from_argv(argc, argv,
                             "Compute a SAXS profile");

    const double taumin=0.01;
    const double taumax=0.2;
    const double qtarget=0.12;
    const unsigned num=100;
    std::ofstream myfile;
    std::clock_t start, end;
    double time;

    IMP_NEW(Model, m, () );
    //setup hierarchy
    Particles particles(get_particles(m));

    //get pdb CA coordinates
    ArrayCoord coords = get_coords(m,particles);

    //get form factors for them
    ArrayXd ffs = get_form_factors(particles);

    //initialize memoized functions with an estimate of dmax
    double dmax = 2*get_distances(coords.row(0), coords).maxCoeff();
    SincFunction sinc(dmax*qtarget, 0.0001);
    SincCosFunction sigma(dmax*qtarget, 0.0001);

    //output: each row is a different tau value
    //columns are:
    //0 exact profile
    //1 analytical covariance
    //2 autocovariance
    //3 cross-covariance
    //4 numerical covariance: 300 iterations
    //5 numerical covariance: 300 iterations
    //6 numerical covariance: 300 iterations
    //7 numerical covariance: 1000 iterations
    ArrayXXd contribs(num,8);
    ArrayXd tauvals(ArrayXd::LinSpaced(num,taumin,taumax));
    for (unsigned i=0; i<num; ++i){
        std::cout << i << std::endl;
        double tau = tauvals(i);

        //compute profile
        contribs(i,0)=tau;
        //compute autocovariance
        contribs(i,2) = compute_autocovariance(coords, ffs, tau, sigma,
                            qtarget, (qtarget+0.1), 2)(0,0);
        //compute crosscovariance
        contribs(i,3) = compute_crosscovariance(coords, ffs, tau, sigma,
                                            qtarget, (qtarget+0.1), 2)(0,0);
        //full matrix
        contribs(i,1) = contribs(i,2)+contribs(i,3);

        //compute perturbed profiles: 300 three times
        contribs(i,4) = compute_covariance_numerical(coords,ffs,tau,sinc,
                qtarget, (qtarget+0.1), 2, 300)(0,0);
        contribs(i,5) = compute_covariance_numerical(coords,ffs,tau,sinc,
                qtarget, (qtarget+0.1), 2, 300)(0,0);
        contribs(i,6) = compute_covariance_numerical(coords,ffs,tau,sinc,
                qtarget, (qtarget+0.1), 2, 300)(0,0);

        //compute perturbed profiles: 1000
        contribs(i,7) = compute_covariance_numerical(coords,ffs,tau,sinc,
                qtarget, (qtarget+0.1), 2, 1000)(0,0);
    }

    myfile.open("contributions.dat");
    myfile << "# " << qtarget << std::endl;
    myfile << contribs << std::endl;
    myfile.close();

    return 0;
}
