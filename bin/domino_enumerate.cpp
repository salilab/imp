/**
 *  \file domino_enumerate.cpp
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/domino_enumerate.h>
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

void add_x_restraint(Model *m, atom::Hierarchy protein, double x0)
{
IMP_NEW(core::Harmonic,ha,(x0,kappa_));
IMP_NEW(core::HarmonicLowerBound,hal,(x0,kappa_));
IMP_NEW(core::AttributeSingletonScore,ass1,(ha,FloatKey("x")));
IMP_NEW(core::AttributeSingletonScore,ass2,(hal,FloatKey("x")));
for(int i=0;i<TM_num;i++)
{
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

void add_y_restraint(Model *m, atom::Hierarchy protein, double x0)
{
IMP_NEW(core::Harmonic,ha,(x0,kappa_));
IMP_NEW(core::AttributeSingletonScore,ass,(ha,FloatKey("y")));
IMP_NEW(container::ListSingletonContainer, lrb, (m));
for(int i=0;i<TM_num;i++)
{
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
for(int i=0;i<TM_num;i++)
{
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
for(int i=0;i<TM_num;i++)
{
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
//add_packing_restraint()
add_DOPE(m,protein);
add_diameter_restraint(m,protein);
add_depth_restraint(m,protein);
//add_tilt_restraint(tilt_range_,rot0)
add_x_restraint(m,protein,0.0);
add_y_restraint(m,protein,0.0);
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
}
return rset.release();
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

m->update();
std::cout << *rset << std::endl;
// create discrete states

// create sampler

// sampling

// writing things to file

return 0;
}
