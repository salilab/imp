/**
 *  \file domino_enumerate.cpp
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/domino_enumerate.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/domino.h>
#include <IMP/membrane.h>
using namespace IMP;
using namespace IMP::membrane;

std::vector<double> generate_TM
(Model *m,atom::Hierarchy *protein,core::TableRefiner *tbr)
{
std::vector<double> rot0;
int nres,jseq;
double x,y,z;
double vol,rg,bb,ee;
algebra::Rotation3D rot;
algebra::Transformation3D tr;

for(int i=0;i<TM_num;i++){
 IMP_NEW(Particle,pm,(m));
 atom::Molecule tm=atom::Molecule::setup_particle(pm);
 tm->set_name(TM_names[i]);
 nres=TM_res[i][1]-TM_res[i][0]+1;
 core::XYZs atoms;
// cycle on the number of residues
 for(int j=0;j<nres;j++){
  x=2.3*cos(radians(100.0)*double(j));
  y=2.3*sin(radians(100.0)*double(j));
  z=1.51*(double(j)-double((nres-1))/2.0);
  // set up residue
  IMP_NEW(Particle,pr,(m));
  jseq=TM_res[i][0]+j;
  atom::Residue
r=atom::Residue::setup_particle(pr,atom::get_residue_type(TM_seq[jseq-1]),jseq);
  vol=atom::get_volume_from_residue_type(r.get_residue_type());
  rg=algebra::get_ball_radius_from_volume_3d(vol);
  //rg=2.273
  //set up atom
  IMP_NEW(Particle,pa,(m));
  atom::Atom a=atom::Atom::setup_particle(pa,atom::AT_CA);
  core::XYZR ad=core::XYZR::setup_particle(pa,
algebra::Sphere3D(algebra::Vector3D(x,y,z),rg));
  r.add_child(a);
  tm.add_child(r);
  atoms.push_back(ad);
 }
 protein->add_child(tm);
 // create rigid body
 IMP_NEW(Particle,prb,(m));
 core::RigidBody rb=core::RigidBody::setup_particle(prb,atoms);
 rb->set_name(TM_names[i]);
 tbr->add_particle(prb,atoms);
 //initialize helix decorator
 bb = (core::RigidMember(atoms[0]).get_internal_coordinates())[0];
 ee = (core::RigidMember(atoms[nres-1]).get_internal_coordinates())[0];
 membrane::HelixDecorator d_rbs=
membrane::HelixDecorator::setup_particle(prb,bb,ee);
 if ( TM_topo[i]*(ee-bb)>0.0 ) rot0.push_back(-IMP::PI/2.0);
 else rot0.push_back(IMP::PI/2.0);
 //initialize system to match topology
 if ( TM_topo[i]<0.0 )
  rot=algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),IMP::PI);
 else rot=algebra::get_identity_rotation_3d();
 tr=algebra::Transformation3D(rot,algebra::Vector3D(double(i)*15.0,0,0));
 core::transform(rb,tr);
}
return rot0;
}

void add_excluded_volume (Model *m, atom::Hierarchy protein)
{
IMP_NEW(container::ListSingletonContainer,lsc,(m));
atom::Selection s=atom::Selection(protein);
s.set_atom_type(atom::AT_CA);
lsc->add_particles(s.get_selected_particles());
IMP_NEW(core::ExcludedVolumeRestraint,evr,(lsc,kappa_));
evr->set_name("Excluded Volume");
m->add_restraint(evr);
m->set_maximum_score(evr, max_score_);
}

void add_DOPE(Model *m, atom::Hierarchy protein)
{
atom::add_dope_score_data(protein);
IMP_NEW(container::ListSingletonContainer,lsc,(m));
atom::Selection s=atom::Selection(protein);
s.set_atom_type(atom::AT_CA);
lsc->add_particles(s.get_selected_particles());
IMP_NEW(container::ClosePairContainer,cpc,(lsc, 15.0));
IMP_NEW(SameHelixPairFilter,f,());
cpc->add_pair_filter(f);
IMP_NEW(atom::DopePairScore,dps,(15.0,atom::get_data_path(score_name_)));
IMP_NEW(container::PairsRestraint,dope,(dps,cpc));
dope->set_name("DOPE scoring function");
m->add_restraint(dope);
m->set_maximum_score(dope, max_score_);
}

void add_packing_restraint
(Model *m,atom::Hierarchy protein,core::TableRefiner *tbr)
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
std::vector<double> om_b, om_e, dd_b, dd_e;

for(int i=0;i<ncl;i++){
 dd_b.push_back(dd0[i]-double(nsig)*sig_dd0[i]);
 dd_e.push_back(dd0[i]+double(nsig)*sig_dd0[i]);
 om_b.push_back(std::max(radians(om0[i]-double(nsig)*sig_om0[i]),-IMP::PI));
 om_e.push_back(std::min(radians(om0[i]+double(nsig)*sig_om0[i]), IMP::PI));
}
IMP_NEW(container::ListSingletonContainer,lrb,(m));
for(int i=0;i<TM_num;i++){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM_names[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 lrb->add_particle(rb);
}
IMP_NEW(container::ClosePairContainer,nrb,(lrb, 25.0));
IMP_NEW(RigidBodyPackingScore,ps,(tbr, om_b, om_e, dd_b, dd_e));
IMP_NEW(container::PairsRestraint,prs,(ps, nrb));
m->add_restraint(prs);
m->set_maximum_score(prs, max_score_);
}

core::PairRestraint* add_distance_restraint
 (Model *m,Particle *s0,Particle *s1,double x0)
{
IMP_NEW(core::HarmonicUpperBound,hub,(x0,kappa_));
IMP_NEW(core::DistancePairScore,df,(hub));
IMP_NEW(core::PairRestraint,dr,(df, ParticlePair(s0, s1)));
dr->set_name("Distance restraint");
m->add_restraint(dr);
m->set_maximum_score(dr, max_score_);
return dr.release();
}

void add_x_restraint(Model *m, atom::Hierarchy protein)
{
IMP_NEW(core::Harmonic,ha,(0.0,kappa_));
IMP_NEW(core::HarmonicLowerBound,hal,(0.0,kappa_));
IMP_NEW(core::AttributeSingletonScore,ass1,(ha,FloatKey("x")));
IMP_NEW(core::AttributeSingletonScore,ass2,(hal,FloatKey("x")));
for(int i=0;i<TM_num;i++){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM_names[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 if( i == 0 ){
  IMP_NEW(core::SingletonRestraint, sr, (ass1, rb));
  sr->set_name("Fix x for particle "+TM_names[i]);
  m->add_restraint(sr);
  m->set_maximum_score(sr, max_score_);
 }
 if( i == 1 ){
  IMP_NEW(core::SingletonRestraint, sr, (ass2, rb));
  sr->set_name("Fix x for particle "+TM_names[i]);
  m->add_restraint(sr);
  m->set_maximum_score(sr, max_score_);
 }
}
}

void add_y_restraint(Model *m, atom::Hierarchy protein)
{
IMP_NEW(core::Harmonic,ha,(0.0,kappa_));
IMP_NEW(core::AttributeSingletonScore,ass,(ha,FloatKey("y")));
IMP_NEW(container::ListSingletonContainer, lrb, (m));
for(int i=0;i<TM_num;i++){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM_names[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 if( i < 2 ) lrb->add_particle(rb);
}
IMP_NEW(container::SingletonsRestraint, sr, (ass, lrb));
sr->set_name("Fix y for at most two particles ");
m->add_restraint(sr);
m->set_maximum_score(sr, max_score_);
}

void add_depth_restraint(Model *m, atom::Hierarchy protein)
{
IMP_NEW(container::ListSingletonContainer, lrb, (m));
for(int i=0;i<TM_num;i++){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM_names[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 lrb->add_particle(rb);
}
IMP_NEW(core::HarmonicWell,well,(z_range_,kappa_));
IMP_NEW(core::AttributeSingletonScore,ass,(well,FloatKey("z")));
IMP_NEW(container::SingletonsRestraint, sr, (ass, lrb));
sr->set_name("Depth restraint");
m->add_restraint(sr);
m->set_maximum_score(sr, max_score_);
}

core::PairRestraint* add_interacting_restraint
(Model *m,Particle *rb0,Particle *rb1,core::TableRefiner *tbr)
{
IMP_NEW(core::HarmonicUpperBound,hub,(d0_inter_, kappa_));
IMP_NEW(core::DistancePairScore,sd,(hub));
IMP_NEW(core::KClosePairsPairScore,kc,(sd,tbr,1));
IMP_NEW(core::PairRestraint,ir,(kc,ParticlePair(rb0,rb1)));
ir->set_name("Interacting restraint");
m->add_restraint(ir);
m->set_maximum_score(ir, max_score_);
return ir.release();
}

void add_diameter_restraint(Model *m, atom::Hierarchy protein)
{
IMP_NEW(container::ListSingletonContainer,lrb,(m));
for(int i=0;i<TM_num;i++){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM_names[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 lrb->add_particle(rb);
}
IMP_NEW(core::HarmonicUpperBound,hub,(0.0,kappa_));
IMP_NEW(core::DiameterRestraint,dr,(hub, lrb, diameter_));
dr->set_name("Diameter restraint");
m->add_restraint(dr);
m->set_maximum_score(dr, max_score_);
}

void add_tilt_restraint(Model *m, atom::Hierarchy protein)
{
Float x0,x1;
algebra::Vector3D laxis=algebra::Vector3D(1.0,0.0,0.0);
algebra::Vector3D zaxis=algebra::Vector3D(0.0,0.0,1.0);
for(int i=0;i<TM_num;i++){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM_names[i]);
 core::RigidBody rb
 =core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
 if ( TM_rot0[i]>0.0 )
      {x0=IMP::PI-tilt_range_.second; x1=IMP::PI-tilt_range_.first;}
 else {x0=tilt_range_.first;          x1=tilt_range_.second;}
 IMP::FloatRange range=IMP::FloatRange(x0,x1);
 IMP_NEW(core::HarmonicWell,well,(range, kappa_));
 IMP_NEW(TiltSingletonScore,tss,(well, laxis, zaxis));
 IMP_NEW(core::SingletonRestraint,sr,(tss, rb));
 m->add_restraint(sr);
 m->set_maximum_score(sr, max_score_);
}
}

RestraintSet* create_restraints
(Model *m,atom::Hierarchy protein,core::TableRefiner *tbr)
{
IMP_NEW(RestraintSet,rset,());
add_excluded_volume(m,protein);
for(int i=0;i<TM_nloop;i++){
    int i0=TM_loop[i][0];
    int i1=TM_loop[i][1];
    atom::Selection s0=atom::Selection(protein);
    s0.set_atom_type(atom::AT_CA);
    s0.set_residue_index(TM_res[i0][1]);
    atom::Selection s1=atom::Selection(protein);
    s1.set_atom_type(atom::AT_CA);
    s1.set_residue_index(TM_res[i1][0]);
    Particle *p0=s0.get_selected_particles()[0];
    Particle *p1=s1.get_selected_particles()[0];
// End-to-End distance restraint
   double length=1.6*(double(TM_res[i1][0]-TM_res[i0][1]+1))+7.4;
   //core::PairRestraint* lr=add_distance_restraint(m,p0,p1,length)
// COM-COM distance restraint
   core::RigidBody rb0=core::RigidMember(p0).get_rigid_body();
   core::RigidBody rb1=core::RigidMember(p1).get_rigid_body();
   core::PairRestraint* lrb=add_distance_restraint(m,rb0,rb1,35.0);
   rset->add_restraint(lrb);
}
add_packing_restraint(m,protein,tbr);
add_DOPE(m,protein);
add_diameter_restraint(m,protein);
add_depth_restraint(m,protein);
add_tilt_restraint(m,protein);
add_x_restraint(m,protein);
add_y_restraint(m,protein);
/*
for(int i=0;i<TM_ninter;i++){
    int i0=TM_inter[i][0];
    int i1=TM_inter[i][1];
    atom::Selection s0=atom::Selection(protein);
    s0.set_molecule(TM_names[i0]);
    atom::Selection s1=atom::Selection(protein);
    s1.set_molecule(TM_names[i1]);
    core::RigidBody rb0
    =core::RigidMember(s0.get_selected_particles()[0]).get_rigid_body();
    core::RigidBody rb1
    =core::RigidMember(s1.get_selected_particles()[0]).get_rigid_body();
    core::PairRestraint* ir=add_interacting_restraint(m,rb0,rb1,tbr);
    rset->add_restraint(ir);
}*/
return rset.release();
}

domino::ParticleStatesTable* create_states(atom::Hierarchy protein)
{
double xx,yy,zz,rg;
algebra::Rotation3D rotz,tilt,rot1,swing,rot2;
algebra::Rotation3D rot0=
algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0), IMP::PI/2.0);
std::vector<algebra::ReferenceFrame3D> trs;
for(int i=-grid_ix;i<grid_ix+1;i++){
 xx=double(i)*grid_Dx;
 for(int j=-grid_iy;j<grid_iy+1;j++){
  yy=double(j)*grid_Dx;
  rg=sqrt(xx*xx+yy*yy);
  if ( rg > diameter_ ) continue;
  for(int k=-grid_iz;k<grid_iz+1;k++){
   zz=double(k)*grid_Dx;
   for(int ii=0;ii<grid_irot;ii++){
    rotz=algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),
double(ii)*grid_Drot);
    for(int jj=0;jj<grid_itilt+1;jj++){
     tilt=algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),
double(jj)*grid_Dtilt);
     rot1 = algebra::compose(tilt,rotz);
     for(int kk=0;kk<grid_iswing;kk++){
      if ( jj == 0  && kk != 0 )  break;
      swing=algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),
double(kk)*grid_Dswing);
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
for(int i=0;i<TM_num;i++){
 atom::Selection s=atom::Selection(protein);
 s.set_molecule(TM_names[i]);
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

int main(int  , char **)
{

// parsing input

// create a new model
IMP_NEW(Model,m,());
// table refiner for rigid bodies
IMP_NEW(core::TableRefiner,tbr,());
// root hierarchy
IMP_NEW(Particle,ph,(m));
atom::Hierarchy all=atom::Hierarchy::setup_particle(ph);

// create representation
TM_rot0=generate_TM(m,&all,tbr);

// create restraints
RestraintSet* rset=create_restraints(m,all,tbr);

// create discrete states
domino::ParticleStatesTable* pst=create_states(all);

// create sampler
domino::DominoSampler* s=create_sampler(m,rset,pst);

// sampling
domino::Subset ass=domino::Subset(pst->get_particles());

domino::Assignments cs=s->get_sample_assignments(ass);
std::cout << "Found " << cs.size() <<" solutions" << std::endl;

// writing things to file

return 0;
}
