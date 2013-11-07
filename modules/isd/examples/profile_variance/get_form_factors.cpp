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
                             "Compute form factors of a PDB file");

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

    ArrayXXd tmp(coords.rows(),4);
    tmp.leftCols<3>() = coords;
    tmp.rightCols<1>() = ffs;
    myfile.open("formfactors.dat");
    myfile << tmp << std::endl;
    myfile.close();
    return 0;
}
