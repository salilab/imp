/**
 *  \file domino_representation.cpp
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/algebra.h>
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>

using namespace IMP;

IMPMEMBRANE_BEGIN_NAMESPACE

core::TableRefiner* generate_TM(Model *m,atom::Hierarchy *protein)
{
int nres,jseq;
double x,y,z;
double vol,rg,bb,ee;
algebra::Rotation3D rot;
algebra::Transformation3D tr;
IMP_NEW(core::TableRefiner,tbr,());

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
 // TableRefiner
 tbr->add_particle(prb,atoms);
 // Check orientation of x-axis and topology
 bb = (core::RigidMember(atoms[0]).get_internal_coordinates())[0];
 ee = (core::RigidMember(atoms[nres-1]).get_internal_coordinates())[0];
 if ( TM_topo[i]*(ee-bb)<0.0 ){
  for(unsigned int i=0;i<atoms.size();i++){
   algebra::Vector3D coord=
    core::RigidMember(atoms[i]).get_internal_coordinates();
   rot=algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),IMP::PI);
   tr=algebra::Transformation3D(rot,algebra::Vector3D(0,0,0));
   core::RigidMember(atoms[i]).set_internal_coordinates
    (tr.get_transformed(coord));
  }
 }
 rb.set_reference_frame(algebra::ReferenceFrame3D(algebra::Transformation3D
      (algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),-IMP::PI/2.0),
       algebra::Vector3D(0,0,0))));
 //initialize helix decorator
 bb = (core::RigidMember(atoms[0]).get_internal_coordinates())[0];
 ee = (core::RigidMember(atoms[nres-1]).get_internal_coordinates())[0];
 membrane::HelixDecorator d_rbs=
  membrane::HelixDecorator::setup_particle(prb,bb,ee);
 }
return tbr.release();
}

IMPMEMBRANE_END_NAMESPACE
