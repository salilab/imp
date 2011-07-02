/**
 *  \file mc_membrane.cpp
 *  \brief Membrane MonteCarlo
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>

using namespace IMP;
using namespace IMP::membrane;

int main(int  , char **)
{

// parsing input
std::cout << "Parsing input file" << std::endl;
Parameters mydata=get_parameters("config.ini");

// create a new model
IMP_NEW(Model,m,());
// root hierarchy
IMP_NEW(Particle,ph,(m));
atom::Hierarchy all=atom::Hierarchy::setup_particle(ph);

// create representation
std::cout << "Creating representation" << std::endl;
core::TableRefiner* tbr=generate_TM(m,all,&mydata);

// create restraints
std::cout << "Creating restraints" << std::endl;
RestraintSet* rset=create_restraints(m,all,tbr,&mydata);

// create sampler
std::cout << "Creating sampler" << std::endl;
core::MonteCarlo* mc=setup_MonteCarlo(m,all,&mydata);

// sampling
std::cout << "Sampling" << std::endl;

return 0;
}
