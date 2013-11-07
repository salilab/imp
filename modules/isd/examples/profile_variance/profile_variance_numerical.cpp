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

    const double tau=0.1;
    const double qmin=0.0;
    const double qmax=0.5;
    const unsigned num=101;
    const unsigned numloops=1000;
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
    SincFunction sinc(dmax*qmax, 0.0001);

    //reference profile
    ArrayXd profile = compute_blurred_profile(coords, ffs, tau, sinc,
            qmin, qmax, num);
    //compute perturbed profiles
    start=clock();
    ArrayXXd covmat = compute_covariance_numerical(coords, ffs,
        tau, sinc, qmin, qmax, num, numloops);
    end=clock();
    time=difftime(end,start)/CLOCKS_PER_SEC;
    //
    //output profile
    std::cout << "profile " << time << " "
        << (time/double(numloops)) << std::endl;
    myfile.open("profiles.dat");
    ArrayXXd residuals(covmat-
            (profile.matrix()*profile.matrix().transpose()).array());
    myfile << residuals << std::endl;
    myfile.close();
    //output covariance matrix
    myfile.open("covmat_num.dat");
    myfile << covmat << std::endl;
    myfile.close();
    //output relative covariance matrix
    myfile.open("covrel_num.dat");
    myfile << make_relative(covmat,profile) << std::endl;
    myfile.close();

    return 0;
}
