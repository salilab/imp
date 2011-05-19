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

int main(int  , char **)
{

// create a new model
IMP_NEW(Model,m,());

// create representation
IMP_NEW(core::TableRefiner,tbr,());
// root hierarchy
IMP_NEW(Particle,ph,(m));
atom::Hierarchy all=atom::Hierarchy::setup_particle(ph);

for(int i=0;i<TM_num;i++){
 IMP_NEW(Particle,pm,(m));
 atom::Molecule tm=atom::Molecule::setup_particle(pm);
 tm->set_name(TM_names[i]);
 int nres=TM_res[i][1]-TM_res[i][0]+1;
 core::XYZs atoms;
// cycle on the number of residues
 for(int j=0;j<nres;j++){
  double x=2.3*cos(radians(100.0)*double(j));
  double y=2.3*sin(radians(100.0)*double(j));
  double z=1.51*(double(j)-double((nres-1))/2.0);
  // set up residue
  IMP_NEW(Particle,pr,(m));
  int jseq=TM_res[i][0]+j;
  atom::Residue
r=atom::Residue::setup_particle(pr,atom::get_residue_type(TM_seq[jseq-1]),jseq);
  double vol=atom::get_volume_from_residue_type(r.get_residue_type());
  double rg=algebra::get_ball_radius_from_volume_3d(vol);
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
 all.add_child(tm);
 // create rigid body
 IMP_NEW(Particle,prb,(m));
 core::RigidBody rb=core::RigidBody::setup_particle(prb,atoms);
 rb->set_name(TM_names[i]);
 tbr->add_particle(prb,atoms);
 //initialize helix decorator
 double bb = (core::RigidMember(atoms[0]).get_internal_coordinates())[0];
 double ee = (core::RigidMember(atoms[nres-1]).get_internal_coordinates())[0];
 membrane::HelixDecorator d_rbs=
membrane::HelixDecorator::setup_particle(prb,bb,ee);
 if ( TM_topo[i]*(ee-bb)>0.0 ) rot0[i]=-IMP::PI/2.0;
 else rot0[i]=IMP::PI/2.0;
 //initialize system to match topology
 algebra::Rotation3D rot;
 if ( TM_topo[i]<0.0 )
rot=algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),IMP::PI);
 else                   rot=algebra::get_identity_rotation_3d();
 algebra::Transformation3D
tr=algebra::Transformation3D(rot,algebra::Vector3D(double(i)*15.0,0,0));
 core::transform(rb,tr);
}

// create restraints

// create discrete states

// create sampler

// sampling

// writing things to file

return 0;
}
