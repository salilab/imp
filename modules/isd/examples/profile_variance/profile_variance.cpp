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
    const double qmin=0.005;
    const double qmax=0.5;
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
    SincFunction sinc(dmax*qmax, 0.0001);
    SincCosFunction sigma(dmax*qmax, 0.0001);

    //compute profile
    start=clock();
    ArrayXd profile=compute_blurred_profile(coords, ffs, tau, sinc,
            qmin, qmax, num);
    end=clock();
    time=difftime(end,start)/CLOCKS_PER_SEC;
    std::cout << "profile " << time << std::endl;
    myfile.open("profile.dat");
    myfile << profile << std::endl;
    myfile.close();

    //compute autocovariance
    start=clock();
    ArrayXXd autocov=compute_autocovariance(coords, ffs, tau, sigma,
                        qmin, qmax, num);
    end=clock();
    time=difftime(end,start)/CLOCKS_PER_SEC;
    ArrayXXd autorel=make_relative(autocov, profile);
    std::cout << "autocov " << time << std::endl;
    myfile.open("autocov.dat");
    myfile << autocov << std::endl;
    myfile.close();
    myfile.open("autorel.dat");
    myfile << autorel << std::endl;
    myfile.close();

    //compute crosscovariance
    start=clock();
    ArrayXXd crosscov=compute_crosscovariance(coords, ffs, tau, sigma,
                                            qmin, qmax, num);
    end=clock();
    time=difftime(end,start)/CLOCKS_PER_SEC;
    ArrayXXd crossrel=make_relative(crosscov, profile);
    std::cout << "crosscov " << time << std::endl;
    myfile.open("crosscov.dat");
    myfile << crosscov << std::endl;
    myfile.close();
    myfile.open("crossrel.dat");
    myfile << crossrel << std::endl;
    myfile.close();

    //compute approx crosscovariance
    ArrayXXd acrosscov=compute_crosscovariance_approx(coords, ffs, tau, sigma,
                                            qmin, qmax, num);
    ArrayXXd acrossrel=make_relative(acrosscov, profile);
    std::cout << "acrosscov" << std::endl;
    myfile.open("acrosscov.dat");
    myfile << acrosscov << std::endl;
    myfile.close();
    myfile.open("acrossrel.dat");
    myfile << acrossrel << std::endl;
    myfile.close();

    //statistics on approximation
    ArrayXXd delta = ((crosscov-acrosscov)/crosscov).abs();
    ArrayXXd deltarel = ((crossrel-acrossrel)/crossrel).abs();
    std::cout << "error abs " << delta.maxCoeff()
        << " " << delta.mean() << std::endl;
    std::cout << "error rel " << deltarel.maxCoeff()
        << " " << deltarel.mean() << std::endl;

    //full matrix
    ArrayXXd covmat = autocov+crosscov;
    ArrayXXd covrel = make_relative(covmat, profile);
    ArrayXXd deltacross = (crosscov/covmat).abs();
    ArrayXXd deltacrossrel = ((covrel-autorel)/covrel).abs();
    std::cout << "absolute cross " << deltacross.maxCoeff()
                << " " << deltacross.mean() << std::endl;
    myfile.open("deltacross.dat");
    myfile << deltacross << std::endl;
    myfile.close();
    std::cout << "relative cross " << deltacrossrel.maxCoeff()
                << " " << deltacrossrel.mean() << std::endl;
    myfile.open("deltacrossrel.dat");
    myfile << deltacrossrel << std::endl;
    myfile.close();
    std::cout << "Variance Matrix " << std::endl;
    myfile.open("covmat.dat");
    myfile << covmat << std::endl;
    myfile.close();
    std::cout << "Relative Variance Matrix " << std::endl;
    myfile.open("covrel.dat");
    myfile << covrel << std::endl;
    myfile.close();

    //diagonal contributions
    ArrayXXd contribs(num,5);
    contribs.col(0) = ArrayXd::LinSpaced(num,qmin,qmax);
    contribs.col(1) = profile;
    contribs.col(2) = covmat.matrix().diagonal().array();
    contribs.col(3) = autocov.matrix().diagonal().array();
    contribs.col(4) = crosscov.matrix().diagonal().array();
    std::cout << "Diagonal contributions " << std::endl;
    myfile.open("contributions.dat");
    myfile << "# " << qmin << " " << qmax << " " << num << std::endl;
    myfile << contribs << std::endl;
    myfile.close();

    return 0;
}
