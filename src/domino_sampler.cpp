/**
 *  \file domino_restraints.cpp
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/domino.h>
#include <IMP/membrane.h>

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

domino::ParticleStatesTable* create_states
(atom::Hierarchy protein, Parameters *myparam)
{

GridParameters *gg=&(myparam->grid);

IMP_NEW(domino::ParticleStatesTable,pst,());

double b[]={-gg->xmax,-gg->xmax,-gg->zmax,0.0,0.0,0.0};
double e[]={gg->xmax,gg->xmax,gg->zmax,gg->rotmax,gg->tiltmax,gg->swingmax};
double bs[]={gg->x,gg->x,gg->x,gg->rot,gg->tilt,gg->swing};
algebra::Vector6D begin(b, b+6), end(e, e+6), binsize(bs, bs+6);

IMP_NEW(membrane::RigidBodyGridStates,rbs,(begin,end,binsize,-IMP::PI/2.0));

for(int i=0;i<myparam->TM.num;i++){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(myparam->TM.name[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 pst->set_particle_states(rb,rbs);
 }
 return pst.release();
}

domino::DominoSampler* create_sampler
(Model *m, RestraintSet *rset, domino::ParticleStatesTable *pst)
{
domino::SubsetFilterTables filters=domino::SubsetFilterTables();
IMP_NEW(domino::DominoSampler,s,(m,pst));
domino::InteractionGraph ig=domino::get_interaction_graph(rset,pst);
domino::SubsetGraph jt=domino::get_junction_tree(ig);
IMP_NEW(domino::ExclusionSubsetFilterTable,esft,(pst));
IMP_NEW(domino::RestraintScoreSubsetFilterTable,rssft,(m,pst));
filters.push_back(esft);
filters.push_back(rssft);
IMP_NEW(domino::BranchAndBoundAssignmentsTable,states,(pst,filters));
s->set_assignments_table(states);
s->set_subset_filter_tables(filters);
return s.release();
}

IMPMEMBRANE_END_NAMESPACE
