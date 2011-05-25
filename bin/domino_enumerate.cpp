/**
 *  \file domino_enumerate.cpp
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/domino.h>
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
core::TableRefiner* tbr=generate_TM(m,all,&(mydata.TM));

// create restraints
std::cout << "Creating restraints" << std::endl;
RestraintSet* rset=create_restraints(m,all,tbr,&mydata);

// create discrete states
std::cout << "Creating sampler" << std::endl;
domino::ParticleStatesTable* pst=create_states(all,&mydata);

// create sampler
domino::DominoSampler* s=create_sampler(m,rset,pst);

// sampling
std::cout << "Sampling" << std::endl;
domino::Subset subs=domino::Subset(pst->get_particles());
domino::Assignments ass=s->get_sample_assignments(subs);
std::cout << "Found " << ass.size() << " solutions" << std::endl;

// write output
std::cout << "Writing output" << std::endl;
write_output(all,pst,&subs,&ass,&mydata);

return 0;
}
