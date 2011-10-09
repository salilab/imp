/**
 *  \file membrane_restraints.cpp
 *  \brief Membrane stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/membrane.h>
#include <IMP/saxs.h>

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

void create_restraints(Model *m,atom::Hierarchy protein,
 core::TableRefiner *tbr,Parameters* myparam) {

HelixData* TM=&(myparam->TM);
RstParameters* RST=&(myparam->RST);

// single-body restraints
for(int i=0;i<TM->num;++i){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM->name[i]);
 core::RigidBody rb=
 core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 if(RST->k_depth>0.)
  add_depth_restraint(m,rb,FloatRange(-RST->zeta,RST->zeta),RST->k_depth);
 if(RST->k_tilt>0.)
  add_tilt_restraint(m,rb,FloatRange(0.0,RST->tilt),RST->k_tilt);
}

// multi-body restraints
if(RST->k_volume>0.) add_excluded_volume(m,protein,RST->k_volume);
if(RST->add_dope)    add_DOPE(m,protein,RST->score_name);
if(RST->k_pack>0.)   add_packing_restraint(m,protein,tbr,TM,RST->k_pack);
if(RST->k_diameter>0.)
 add_diameter_restraint(m,protein,RST->diameter,TM,RST->k_diameter);
if(RST->k_saxs>0.)
 add_saxs_restraint(m,protein,RST->saxs_profile,RST->k_saxs);

// two-body restraints
for(unsigned int i=0;i<TM->loop.size();++i){
 int i0=TM->loop[i].first;
 int i1=TM->loop[i].second;
 atom::Selection s0=atom::Selection(protein);
 s0.set_atom_type(atom::AT_CA);
 s0.set_residue_index(TM->resid[i0].second);
 atom::Selection s1=atom::Selection(protein);
 s1.set_atom_type(atom::AT_CA);
 s1.set_residue_index(TM->resid[i1].first);
 Particle *p0=s0.get_selected_particles()[0];
 Particle *p1=s1.get_selected_particles()[0];
// End-to-End distance restraint
 double length=1.6*(double(TM->resid[i1].first-TM->resid[i0].second+1))+7.4;
 if(RST->k_endtoend>0.) add_distance_restraint(m,p0,p1,length,RST->k_endtoend);
// COM-COM distance restraint
 core::RigidBody rb0=core::RigidMember(p0).get_rigid_body();
 core::RigidBody rb1=core::RigidMember(p1).get_rigid_body();
 if(RST->k_cmdist>0.)
  add_distance_restraint(m,rb0,rb1,RST->cm_dist,RST->k_cmdist);
}
for(unsigned int i=0;i<TM->inter.size();++i){
 int i0=TM->inter[i].first;
 int i1=TM->inter[i].second;
 atom::Selection s0=atom::Selection(protein);
 s0.set_molecule(TM->name[i0]);
 atom::Selection s1=atom::Selection(protein);
 s1.set_molecule(TM->name[i1]);
 core::RigidBody rb0=
 core::RigidMember(s0.get_selected_particles()[0]).get_rigid_body();
 core::RigidBody rb1=
 core::RigidMember(s1.get_selected_particles()[0]).get_rigid_body();
 if(RST->k_inter>0.)
  add_interacting_restraint(m,rb0,rb1,tbr,RST->d0_inter,RST->k_inter);
}
}

void add_excluded_volume(Model *m,atom::Hierarchy protein,double kappa)
{
IMP_NEW(container::ListSingletonContainer,lsc,(m));
atom::Selection s=atom::Selection(protein);
s.set_atom_type(atom::AT_CA);
lsc->add_particles(s.get_selected_particles());
IMP_NEW(core::ExcludedVolumeRestraint,evr,(lsc,kappa));
evr->set_name("Excluded Volume");
m->add_restraint(evr);
}

void add_DOPE(Model *m,atom::Hierarchy protein,std::string sname)
{
atom::add_dope_score_data(protein);
IMP_NEW(container::ListSingletonContainer,lsc,(m));
atom::Selection s=atom::Selection(protein);
s.set_atom_type(atom::AT_CA);
lsc->add_particles(s.get_selected_particles());
IMP_NEW(container::ClosePairContainer,cpc,(lsc, 15.0));
IMP_NEW(SameHelixPairFilter,f,());
cpc->add_pair_filter(f);
IMP_NEW(atom::DopePairScore,dps,(15.0,atom::get_data_path(sname)));
IMP_NEW(container::PairsRestraint,dope,(dps,cpc));
dope->set_name("DOPE scoring function");
m->add_restraint(dope);
}

void add_packing_restraint
(Model *m,atom::Hierarchy protein,core::TableRefiner *tbr,
 HelixData *TM,double kappa)
{
//if the helices are interacting, apply a filter on the crossing angle
//first define the allowed intervals, by specifying the center
//of the distributions (Walters and DeGrado PNAS (2007) 103:13658)
const double om0[]={-156.5, 146.4, -37.9, 13.8, 178.0, 25.5, -161.1, 44.8,
127.4, -60.2, -129.2, 2.4, 161.0};
//the sigmas
const double sig_om0[]={10.1, 13.6, 7.50, 16.6, 20.8, 11.2, 10.3, 8.8, 12.3,
14.8, 12.9, 16.2, 17.6};
//  distance cutoff
const double dd0[]={8.61, 8.57, 7.93, 9.77, 9.14, 8.55, 9.30, 7.96, 9.40, 8.61,
8.97, 8.55, 8.75};
//  and distance sigmas
const double sig_dd0[]={0.89, 0.99, 0.88, 1.18, 1.47, 1.05, 1.57, 1.13, 1.0,
1.04, 1.65, 0.78, 1.33};
//  the allowed number of sigma
int nsig=packing_nsig_;
//  and the number of clusters
int ncl=packing_ncl_;
// create allowed intervals (omega in radians)
Floats om_b, om_e, dd_b, dd_e;

for(int i=0;i<ncl;++i){
 dd_b.push_back(dd0[i]-double(nsig)*sig_dd0[i]);
 dd_e.push_back(dd0[i]+double(nsig)*sig_dd0[i]);
 om_b.push_back(std::max(radians(om0[i]-double(nsig)*sig_om0[i]),-IMP::PI));
 om_e.push_back(std::min(radians(om0[i]+double(nsig)*sig_om0[i]), IMP::PI));
}
IMP_NEW(container::ListSingletonContainer,lrb,(m));
for(int i=0;i<TM->num;++i){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM->name[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 lrb->add_particle(rb);
}
IMP_NEW(container::ClosePairContainer,nrb,(lrb, 25.0));
IMP_NEW(RigidBodyPackingScore,ps,(tbr,om_b,om_e,dd_b,dd_e,kappa));
IMP_NEW(container::PairsRestraint,prs,(ps,nrb));
prs->set_name("Packing restraint");
m->add_restraint(prs);
}

void add_distance_restraint
 (Model *m,Particle *p0,Particle *p1,double x0,double kappa)
{
IMP_NEW(core::HarmonicUpperBound,hub,(x0,kappa));
IMP_NEW(core::DistancePairScore,df,(hub));
IMP_NEW(core::PairRestraint,dr,(df,ParticlePair(p0,p1)));
dr->set_name("Distance restraint");
m->add_restraint(dr);
}

void add_depth_restraint
 (Model *m, Particle *p,FloatRange z_range,double kappa)
{
IMP_NEW(core::HarmonicWell,well,(z_range,kappa));
IMP_NEW(core::AttributeSingletonScore,ass,(well,FloatKey("z")));
IMP_NEW(core::SingletonRestraint,sr,(ass,p));
sr->set_name("Depth restraint");
m->add_restraint(sr);
}

void add_tilt_restraint
 (Model *m,Particle *p,FloatRange tilt_range,double kappa)
{
algebra::Vector3D laxis=algebra::Vector3D(1.0,0.0,0.0);
algebra::Vector3D zaxis=algebra::Vector3D(0.0,0.0,1.0);
IMP_NEW(core::HarmonicWell,well,(tilt_range, kappa));
IMP_NEW(TiltSingletonScore,tss,(well,laxis,zaxis));
IMP_NEW(core::SingletonRestraint,sr,(tss,p));
m->add_restraint(sr);
sr->set_name("Tilt restraint");
}

void add_interacting_restraint
(Model *m,Particle *rb0,Particle *rb1,core::TableRefiner *tbr,
double d0_inter,double kappa)
{
IMP_NEW(core::HarmonicUpperBound,hub,(d0_inter, kappa));
IMP_NEW(core::DistancePairScore,sd,(hub));
IMP_NEW(core::KClosePairsPairScore,kc,(sd,tbr,1));
IMP_NEW(core::PairRestraint,ir,(kc,ParticlePair(rb0,rb1)));
ir->set_name("Interacting restraint");
m->add_restraint(ir);
}

void add_diameter_restraint
(Model *m,atom::Hierarchy protein,double diameter,
 HelixData *TM,double kappa)
{
IMP_NEW(container::ListSingletonContainer,lrb,(m));
for(int i=0;i<TM->num;++i){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM->name[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 lrb->add_particle(rb);
}
IMP_NEW(container::AllPairContainer,apc,(lrb));
IMP_NEW(core::HarmonicUpperBound,hub,(diameter,kappa));
IMP_NEW(core::DistancePairScore,dps,(hub));
IMP_NEW(container::PairsRestraint,dr,(dps,apc));
dr->set_name("Diameter restraint");
m->add_restraint(dr);
}

void add_saxs_restraint
 (Model *m,atom::Hierarchy protein,std::string saxs_profile,double kappa)
{
 saxs::Profile profile = saxs::Profile(saxs_profile);
 saxs::FormFactorType ff_type = saxs::CA_ATOMS;
 atom::Selection s=atom::Selection(protein);
 s.set_atom_type(atom::AT_CA);
 Particles ps=s.get_selected_particles();
 IMP_NEW(saxs::Restraint,sr,(ps,profile,ff_type));
 sr->set_name("SAXS restraint");
 m->add_restraint(sr);
}

IMPMEMBRANE_END_NAMESPACE
