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
#include <IMP/rmf.h>

using namespace IMP;
using namespace IMP::membrane;

int main(int  , char **)
{

// parsing input
Parameters mydata=get_parameters("config.ini");

// create a new model
IMP_NEW(Model,m,());
// root hierarchy
IMP_NEW(Particle,ph,(m));
atom::Hierarchy all=atom::Hierarchy::setup_particle(ph);

// create representation
core::TableRefiner* tbr=generate_TM(m,&all,&(mydata.TM));

// create restraints
RestraintSet* rset=create_restraints(m,all,tbr,&mydata);

// create discrete states
domino::ParticleStatesTable* pst=create_states(all,&mydata);

// create sampler
domino::DominoSampler* s=create_sampler(m,rset,pst);

// sampling
domino::Subset ass=domino::Subset(pst->get_particles());
domino::Assignments cs=s->get_sample_assignments(ass);
std::cout << "Found " << cs.size() << " solutions" << std::endl;

// writing things to file
//rmf::HDF5Group rt= rmf::HDF5Group(mydata.ass_file, true);
//rmf::HDF5Group rt= rmf::HDF5Group(mydata.ass_file, false);

//rmf::HDF5DataSet<rmf::IndexTraits> data_set=
//rt.add_child_index_data_set("node_1_assignments", 2);
//rmf::HDF5DataSet<rmf::IndexTraits> data_set=
//rt.get_child_index_data_set("node_1_assignments", 2);

//domino::set_assignments(data_set, cs, ass, pst->get_particles());
//domino::Assignments cs=
//domino::get_assignments(data_set, ass, pst->get_particles());

//std::cout << "Found " << cs.size() << " solutions" << std::endl;

return 0;
}
