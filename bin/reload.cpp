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
#include <IMP/rmf.h>
#include <boost/scoped_array.hpp>
#include <time.h>
#include <iostream>

using namespace IMP;
using namespace IMP::membrane;


int main(int argc, char* argv[])
{

// parsing input
Parameters mydata=get_parameters("config.ini");

// create a new model
IMP_NEW(Model,m,());
// root hierarchy
IMP_NEW(Particle,ph,(m));
atom::Hierarchy all=atom::Hierarchy::setup_particle(ph);

// create representation
core::TableRefiner* tbr=generate_TM(m,all,&mydata);

// trajectory file
RMF::RootHandle rh = RMF::open_rmf_file(mydata.traj_file);
atom::Hierarchies hs=all.get_children();
rmf::set_hierarchies(rh, hs);

// number of frames
unsigned int nframes=rmf::get_number_of_frames(rh,hs[0]);

// create restraints
create_restraints(m,all,tbr,&mydata);

// Monte Carlo loop
for(int imc=0;imc<nframes;++imc)
{

// load configuration from file
  for(int i=0;i<hs.size();++i){
   rmf::load_frame(rh,imc,hs[i]);
  }

// print statistics
 double myscore=m->evaluate(false);
 std::cout << imc << " " << myscore << std::endl;

}

return 0;
}
