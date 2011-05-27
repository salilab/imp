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
#include <math.h>

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

domino::ListSubsetFilterTable* create_states
(atom::Hierarchy protein,Parameters *myparam,domino::ParticleStatesTable* pst)
{
GridParameters *gg=&(myparam->grid);
//double b[]={-gg->xmax,-gg->xmax,-gg->zmax,0.0,0.0,0.0};
//double e[]={gg->xmax,gg->xmax,gg->zmax,gg->rotmax,gg->tiltmax,gg->swingmax};
double bs[]={gg->x,gg->x,gg->x,gg->rot,gg->tilt,gg->swing};
//algebra::Vector6D begin(b, b+6), end(e, e+6), binsize(bs, bs+6);

for(int i=0;i<myparam->TM.num;++i){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(myparam->TM.name[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 if ( i == 0 ){
  double b[]={0.0,0.0,-gg->zmax,0.0,0.0,0.0};
  double e[]={0.0,0.0,gg->zmax,gg->rotmax,gg->tiltmax,gg->swingmax};
  algebra::Vector6D begin(b, b+6), end(e, e+6), binsize(bs, bs+6);
  IMP_NEW(membrane::RigidBodyGridStates,rbs,(begin,end,binsize,-IMP::PI/2.0));
  pst->set_particle_states(rb,rbs);
 }
if ( i == 1 ){
  double b[]={0.0,0.0,-gg->zmax,0.0,0.0,0.0};
  double e[]={gg->xmax,0.0,gg->zmax,gg->rotmax,gg->tiltmax,gg->swingmax};
  algebra::Vector6D begin(b, b+6), end(e, e+6), binsize(bs, bs+6);
  IMP_NEW(membrane::RigidBodyGridStates,rbs,(begin,end,binsize,-IMP::PI/2.0));
  pst->set_particle_states(rb,rbs);
 }
if ( i > 1 ){
  double b[]={-gg->xmax,-gg->xmax,-gg->zmax,0.0,0.0,0.0};
  double e[]={gg->xmax,gg->xmax,gg->zmax,gg->rotmax,gg->tiltmax,gg->swingmax};
  algebra::Vector6D begin(b, b+6), end(e, e+6), binsize(bs, bs+6);
  IMP_NEW(membrane::RigidBodyGridStates,rbs,(begin,end,binsize,-IMP::PI/2.0));
  pst->set_particle_states(rb,rbs);
 }
}

IMP_NEW(domino::ListSubsetFilterTable,lsft,(pst));

/*
IMP_NEW(membrane::RigidBodyGridStates,rbs,(begin,end,binsize,-IMP::PI/2.0));

for(int i=0;i<myparam->TM.num;++i){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(myparam->TM.name[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 pst->set_particle_states(rb,rbs);
}

IMP_NEW(domino::ListSubsetFilterTable,lsft,(pst));

for(int i=0;i<myparam->TM.num;++i){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(myparam->TM.name[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 Ints states;
 unsigned int nstates=rbs->get_number_of_particle_states();
 for(unsigned int j=0;j<nstates;++j){
  Ints nindex = rbs->get_index(j);
  double xx=double(nindex[0])*gg->x-gg->xmax;
  double yy=double(nindex[1])*gg->x-gg->xmax;
  if( i==0 && fabs(xx)<0.0001 && fabs(yy)<0.0001) states.push_back(j);
  if( i==1 && xx>0.0          && fabs(yy)<0.0001) states.push_back(j);
  if( i > 1 ) states.push_back(j);
 }
 lsft->set_allowed_states(rb, states);
}
*/
return lsft.release();
}

domino::DominoSampler* create_sampler
(Model *m, RestraintSet *rset, domino::ParticleStatesTable *pst,
 domino::ListSubsetFilterTable* lsft)
{
IMP_NEW(domino::DominoSampler,s,(m,pst));
// interaction graph, junction tree and merge tree
domino::InteractionGraph ig=domino::get_interaction_graph(rset,pst);
domino::SubsetGraph      jt=domino::get_junction_tree(ig);
domino::MergeTree        mt=domino::get_merge_tree(jt);
s->set_merge_tree(mt);
// filters
domino::SubsetFilterTables filters=domino::SubsetFilterTables();
IMP_NEW(domino::ExclusionSubsetFilterTable,esft,(pst));
IMP_NEW(domino::RestraintScoreSubsetFilterTable,rssft,(m,pst));
rssft->set_use_caching(false);
filters.push_back(esft);
filters.push_back(rssft);
//filters.push_back(lsft);
IMP_NEW(domino::BranchAndBoundAssignmentsTable,states,(pst,filters));
s->set_assignments_table(states);
s->set_subset_filter_tables(filters);
return s.release();
}

IMPMEMBRANE_END_NAMESPACE
