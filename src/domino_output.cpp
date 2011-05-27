/**
 *  \file domino_output.cpp
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/rmf.h>
#include <IMP/domino.h>
#include <IMP/membrane.h>

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

void write_output(atom::Hierarchy protein,
domino::ParticleStatesTable* pst, domino::Subset* subs,
domino::Assignments* ass, Parameters *myparam)
{
// writing assignments
//if(myparam->ass_file.length()>0.0){
// rmf::HDF5Group rt= rmf::HDF5Group(myparam->ass_file, true);
// rmf::HDF5DataSet<rmf::IndexTraits> data_set=
// rt.add_child_index_data_set("node_assignments", 2);
// domino::set_assignments(data_set, *ass, *subs, pst->get_particles());
//}

// writing configurations
if(myparam->traj_file.length()>0.0){
 rmf::RootHandle rh=rmf::RootHandle(myparam->traj_file, true);
 atom::HierarchiesTemp hs = protein.get_children();
 for(unsigned int i=0;i<hs.size();++i){
  rmf::add_hierarchy(rh, hs[i]);
 }
 for(unsigned int i=0;i<ass->size();++i){
  domino::load_particle_states(*subs,(*ass)[i],pst);
  for(unsigned int j=0;j<hs.size();++j){
   rmf::save_frame(rh, i, hs[j]);
  }
 }
}
}

IMPMEMBRANE_END_NAMESPACE
