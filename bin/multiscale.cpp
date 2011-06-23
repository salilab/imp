/**
 *  \file multiscale.cpp
 *  \brief Multiscale 2D sampling
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <IMP/base_types.h>
#include <IMP/domino.h>
#include <string>
#include <iostream>

using namespace IMP;


core::PairRestraint* add_distance_restraint
 (Model *m, Particle *p0, Particle *p1,
  double x0, std::string name, double max_score)
{
 IMP_NEW(core::HarmonicUpperBound,hub,(x0,1.0));
 IMP_NEW(core::DistancePairScore,df,(hub));
 IMP_NEW(core::PairRestraint,dr,(df, ParticlePair(p0, p1)));
 dr->set_name(name);
 m->add_restraint(dr);
 m->set_maximum_score(dr, max_score);
 return dr.release();
}

void add_diameter_restraint
(Model *m, Particles all,
 double diameter, double max_score)
{
 int n=all.size();
 for(int i=0;i<n-1;++i){
  for(int j=i+1;j<n;++j){
    IMP_NEW(core::HarmonicUpperBound,hub,(diameter,1.0));
    IMP_NEW(core::DistancePairScore,df,(hub));
    IMP_NEW(core::PairRestraint,dr,(df, ParticlePair(all[i], all[j])));
    std::string name0=core::XYZ(all[i])->get_name();
    std::string name1=core::XYZ(all[j])->get_name();
    dr->set_name("Diameter"+name0+"-"+name1);
    m->add_restraint(dr);
    m->set_maximum_score(dr, max_score);
  }
 }
}

domino::ListSubsetFilterTable* setup
(Model *m, Particles all, RestraintSet *rset,
domino::ParticleStatesTable *pst, domino::DominoSampler *s,
algebra::Vector3Ds cover, algebra::Vector3Ds cover_x, double scale)
{
 IMP_NEW(domino::XYZStates,st1,(cover_x));
 IMP_NEW(domino::XYZStates,st2,(cover));
 algebra::Vector3Ds origin = algebra::Vector3Ds();
 origin.push_back(algebra::Vector3D(0.0,0.0,0.0));
 IMP_NEW(domino::XYZStates,st0,(origin));

 for(int i=0;i<all.size();++i){
  if(i==0) pst->set_particle_states(all[i],st0);
  if(i==1) pst->set_particle_states(all[i],st1);
  if(i>1)  pst->set_particle_states(all[i],st2);
 }

 RestraintSet *rs=m->get_root_restraint_set();
 for(unsigned int i=0;i<rs->get_number_of_restraints();++i)
  rs->get_restraint(i)->set_maximum_score(0.9*scale*scale);

 IMP_NEW(domino::ListSubsetFilterTable,lsft,(pst));
 IMP_NEW(domino::RestraintScoreSubsetFilterTable,rssft,(m, pst));
 domino::SubsetFilterTables fs=domino::SubsetFilterTables();
 fs.push_back(lsft);
 fs.push_back(rssft);
 domino::InteractionGraph ig=domino::get_interaction_graph(rset,pst);
 domino::SubsetGraph      jt=domino::get_junction_tree(ig);
 s->set_subset_filter_tables(fs);
 return lsft.release();
}

std::vector< std::vector <unsigned int> > get_mapping
(algebra::Vector3Ds cover0, algebra::Vector3Ds cover1)
{
 IMP_NEW(algebra::NearestNeighbor3D,nn,(cover0));
 std::vector< std::vector <unsigned int> > ret;
 ret.resize(cover0.size());

 for(unsigned int i=0;i<cover1.size();++i){
  unsigned int nns = nn->get_nearest_neighbor(cover1[i]);
  ret[nns].push_back(i);
 }
 return ret;
}

int main(int  , char **)
{

IMP_NEW(Model,m,());
double ds=20.0;
double max_score_=0.9*ds*ds;
int    niter=2;
int    nTMH=7;
Particles all;

std::cout << "creating representation" << std::endl;
for(int i=0;i<nTMH;++i){
    IMP_NEW(Particle,p,(m));
    core::XYZ xyz=core::XYZ::setup_particle(p, algebra::Vector3D(0,0,0));
    std::stringstream ss;
    ss << i;
    xyz->set_name("TM"+ss.str());
    all.push_back(p);
}

std::cout << "creating restraints" << std::endl;

IMP_NEW(RestraintSet,rset,());
//distance between adjacent helices
for(int i=0;i<nTMH-1;++i){
 std::stringstream ss1, ss2;
 ss1 << i;
 ss2 << i+1;
 core::PairRestraint* dr=add_distance_restraint(m,all[i],all[i+1],35.0,
  "distance TM"+ss1.str()+"-TM"+ss2.str(),max_score_);
 rset->add_restraint(dr);
}

//distance between interacting helices
std::vector<std::pair<int,int> >  TM_inter;
TM_inter.push_back(std::pair<int,int> (0,1));
TM_inter.push_back(std::pair<int,int> (0,6));
TM_inter.push_back(std::pair<int,int> (1,6));
TM_inter.push_back(std::pair<int,int> (5,6));

for(int i=0;i<TM_inter.size();++i){
 int i0=TM_inter[i].first;
 int i1=TM_inter[i].second;
 std::string name0=all[i0]->get_name();
 std::string name1=all[i1]->get_name();
 core::PairRestraint* dr=
 add_distance_restraint(m,all[i0],all[i1],20.0,
  "interaction "+name0+"-"+name1,max_score_);
 rset->add_restraint(dr);
}

// diameter
//nTMH::  2   3   4   5   6   7   8   9   10   11   12   13   14
// d   :: 16  35  26  51  48  51  50  34   39   52   48   43   42
double diameter=50.0;
add_diameter_restraint(m,all,diameter,max_score_);

std::cout << "creating states" << std::endl;
algebra::BoundingBox2D bb=
algebra::BoundingBox2D(algebra::Vector2D(-diameter,-diameter),
                       algebra::Vector2D(diameter,diameter));

algebra::BoundingBox2D bbx=
algebra::BoundingBox2D(algebra::Vector2D(-diameter,0.0),
                       algebra::Vector2D(diameter,0.0));


std::vector<algebra::Vector3Ds> cover, cover_x;
for(int i=0;i<niter;++i)
{
 std::vector<algebra::Vector2D> cur=
 algebra::get_grid_interior_cover_by_spacing(bb, ds/pow(2,i));
 algebra::Vector3Ds grid=algebra::Vector3Ds();
 for(int j=0;j<cur.size();++j)
  grid.push_back(algebra::Vector3D((cur[j])[0],(cur[j])[1],0.0));
 cover.push_back(grid);

 std::vector<algebra::Vector2D> cur_x=
 algebra::get_grid_interior_cover_by_spacing(bbx, ds/pow(2,i));
 algebra::Vector3Ds grid_x=algebra::Vector3Ds();
 for(int j=0;j<cur_x.size();++j)
  if((cur_x[j])[0]>=0.0)
   grid_x.push_back(algebra::Vector3D((cur_x[j])[0],(cur_x[j])[1],0.0));
 cover_x.push_back(grid_x);
}

// let's go with domino

IMP_NEW(domino::ParticleStatesTable,pst,());
IMP_NEW(domino::DominoSampler,s,(m,pst));

domino::ListSubsetFilterTable* lf=setup
(m,all,rset,pst,s,cover[0],cover_x[0],ds);

domino::Subset subs=domino::Subset(pst->get_particles());
domino::Assignments ass=s->get_sample_assignments(subs);

std::cout << "for scale " << ds << " got " << ass.size() <<
 " out of " << pow(cover[0].size(),nTMH-2)*cover_x[0].size() << std::endl;

return 0;
}
