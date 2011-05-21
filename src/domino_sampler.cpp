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
double xx,yy,zz;
algebra::Rotation3D rotz,tilt,rot1,swing,rot2;
algebra::Rotation3D rot0=
algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0), -IMP::PI/2.0);
algebra::ReferenceFrame3Ds trs=algebra::ReferenceFrame3Ds();
GridParameters *grid=&(myparam->grid);

//grid parameters
int ix     = int(grid->xmax/grid->x);
int iy     = int(grid->xmax/grid->x);
int iz     = int(grid->zmax/grid->x);
int irot   = int(grid->rotmax/grid->rot);
int itilt  = int(grid->tiltmax/grid->tilt);
int iswing = int(grid->swingmax/grid->swing);

for(int i=-ix;i<ix+1;++i){
 xx=double(i)*grid->x;
 for(int j=-iy;j<iy+1;++j){
  yy=double(j)*grid->x;
  double rg=sqrt(xx*xx+yy*yy);
  if ( rg > myparam->diameter ) continue;
  for(int k=-iz;k<iz+1;++k){
   zz=double(k)*grid->x;
   for(int ii=0;ii<irot;++ii){
    rotz=algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),
double(ii)*grid->rot);
    for(int jj=0;jj<itilt+1;++jj){
     tilt=algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),
double(jj)*grid->tilt);
     rot1 = algebra::compose(tilt,rotz);
     for(int kk=0;kk<iswing;++kk){
      if ( jj == 0  && kk != 0 )  break;
      swing=algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),
double(kk)*grid->swing);
      rot2=algebra::compose(swing,rot1);
      algebra::ReferenceFrame3D frame=
      algebra::ReferenceFrame3D(algebra::Transformation3D
      (algebra::compose(rot2,rot0),algebra::Vector3D(xx,yy,zz)));
      trs.push_back(frame);
     }
    }
   }
  }
 }
}

IMP_NEW(domino::ParticleStatesTable,pst,());
IMP_NEW(domino::RigidBodyStates,rbs,(trs));
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
