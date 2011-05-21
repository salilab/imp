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

void write_output
(domino::ParticleStatesTable* pst, domino::Subset* subs,
 domino::Assignments* ass, Parameters *myparam)
{
// assignments file
if(myparam->ass_file.length()>0.0){
 rmf::HDF5Group rt= rmf::HDF5Group(myparam->ass_file, true);
 rmf::HDF5DataSet<rmf::IndexTraits> data_set=
rt.add_child_index_data_set("node_assignments", 2);
 domino::set_assignments(data_set, *ass, *subs, pst->get_particles());
}

// configurations
if(myparam->traj_file.length()>0.0){
}
}

IMPMEMBRANE_END_NAMESPACE
