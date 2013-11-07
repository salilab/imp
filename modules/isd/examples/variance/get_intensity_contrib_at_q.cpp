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
#include <algorithm>

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
    const double q=0.4;
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
    SincFunction sf(dmax*q, 0.0001);

    //I(q) = E^2(q) sum_{i<=j} fi(0) fj(0) sinc(q dij)
    //E^2(q) = exp(- q^2 * 0.23 )
    //data is: fi fj dij fi*fj*sinc(qdij)
    std::vector<std::vector<double> > data;

    //autocorrelation
    for (unsigned i=0; i<ffs.rows(); ++i){
        std::vector<double> datum;
        datum.push_back(ffs(i));
        datum.push_back(ffs(i));
        datum.push_back(0.);
        datum.push_back(square(ffs(i)));
        data.push_back(datum);
    }

    //other terms
    for (unsigned i=0; i<ffs.rows()-1; ++i){
        ArrayXd dists = get_distances(coords.row(i),
                coords.bottomRows(ffs.rows()-i-1));
        for (unsigned j=0; j<dists.rows(); ++j){
            double dij = dists(j);
            double fij = 2*ffs(i)*ffs(i+1+j);
            double sinc = std::abs(sf.sinc(q*dij));
            std::vector<double> datum;
            datum.push_back(ffs(i));
            datum.push_back(ffs(i+1+j));
            datum.push_back(dij);
            datum.push_back(fij*sinc);
            data.push_back(datum);
        }
    }

    //sort all terms wrt last column
    std::sort(data.begin(), data.end(), compfunction);

    //normalize all terms
    for (unsigned i=0; i<data.size(); i++)
        data[i][3] = data[i][3]/data[0][3];

    //write output
    myfile.open("contributions.dat");
    myfile << "# " << q << std::endl;
    for (unsigned i=0; i<data.size(); i++)
        myfile << data[i][0] << " "
               << data[i][1] << " "
               << data[i][2] << " "
               << data[i][3]
               << std::endl;
    myfile.close();

    return 0;
}
