/**
 *  \file spb_representation.cpp
 *  \brief SPB Representation
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <IMP/membrane.h>
#include <IMP/display.h>
#include <string>
using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

atom::Hierarchies create_hierarchies(Model *m,int ncells,std::string name)
{
 atom::Hierarchies hs=atom::Hierarchies();
 for(int i=0;i<ncells;++i){
  IMP_NEW(Particle,p,(m));
  atom::Hierarchy h=atom::Hierarchy::setup_particle(p);
  std::stringstream out;
  out << i;
  h->set_name(name+" hierarchy, cell " + out.str());
  hs.push_back(h);
 }
 return hs;
}

atom::Molecule create_protein(Model *m,std::string name,double mass,int nbeads,
 display::Color colore,int copy,double kappa,
 algebra::Vector3D x0, int start_residue,int length)
{
 if(length==-1) {length=(int) (mass*1000.0/110.0);}
 IMP_NEW(Particle,p,(m));
 atom::Molecule protein=atom::Molecule::setup_particle(p);
 protein->set_name(name);
 double vol=atom::get_volume_from_mass(1000.0*mass)/(double)nbeads;
 double ms=1000.0*mass/(double)nbeads;
 double rg=algebra::get_ball_radius_from_volume_3d(vol);
 for(int i=0;i<nbeads;++i){
  IMP_NEW(Particle,pp,(m));
  int first=start_residue+i*(int)(length/nbeads);
  int last=start_residue+(i+1)*(int)(length/nbeads);
  std::stringstream out1,out2;
  out1 << i;
  out2 << copy;
  atom::Domain dom=atom::Domain::setup_particle(pp, IntRange(first, last));
  dom->set_name(name+out1.str()+"-"+out2.str());
  core::XYZR  d=core::XYZR::setup_particle(pp);
  d.set_radius(rg);
  d.set_coordinates(x0);
  d.set_coordinates_are_optimized(true);
  atom::Mass mm=atom::Mass::setup_particle(pp,ms);
  display::Colored cc=display::Colored::setup_particle(pp,colore);
  protein.add_child(dom);
 }
 if(nbeads>1 && copy==0){
  atom::Selections ss=atom::Selections();
  atom::Hierarchies hs=protein.get_children();
  for(unsigned int i=0;i<hs.size();++i){
   ss.push_back(atom::Selection(hs[i]));
  }
  Restraint *con=atom::create_connectivity_restraint(ss,kappa);
  con->set_name("Connectivity Restraint for "+name);
  m->add_restraint(con);
 }
 return protein;
}

atom::Molecule create_protein(Model *m,std::string name,
 std::string filename,int nbeads,display::Color colore,int copy,
 algebra::Vector3D x0,int start_residue)
{
 IMP_NEW(Particle,p,(m));
 atom::Molecule protein=atom::Molecule::setup_particle(p);
 protein->set_name(name);
 IMP_NEW(atom::CAlphaPDBSelector,sel,());
 atom::Hierarchy hpdb=atom::read_pdb(filename,m,sel);
 Particles ps=atom::get_leaves(hpdb);
 int nres=ps.size();
 core::XYZRs rbps;
 for(int i=0;i<nbeads;++i){
  IMP_NEW(Particle,pp,(m));
  int first=start_residue+i*(int)(nres/nbeads);
  int last=start_residue+(i+1)*(int)(nres/nbeads);
  std::stringstream out1,out2;
  out1 << i;
  out2 << copy;
  atom::Domain dom=atom::Domain::setup_particle(pp, IntRange(first, last));
  dom->set_name(name+out1.str()+"-"+out2.str());
  core::XYZR  d=core::XYZR::setup_particle(pp);
// calculate enclosing sphere and mass
  double ms=0.0;
  core::XYZRs xyz;
  for(int j=i*(int)(nres/nbeads);j<(i+1)*(int)(nres/nbeads);++j){
   xyz.push_back(core::XYZR(ps[j]));
   ms+=atom::Mass(ps[j]).get_mass();
  }
  algebra::Sphere3D sph=core::get_enclosing_sphere(xyz);
  d.set_radius(sph.get_radius());
  d.set_coordinates(sph.get_center());
  d.set_coordinates_are_optimized(true);
  atom::Mass mm=atom::Mass::setup_particle(pp,ms);
  display::Colored cc=display::Colored::setup_particle(pp,colore);
  protein.add_child(dom);
  rbps.push_back(d);
 }
 atom::destroy(hpdb);
 IMP_NEW(Particle,prb,(m));
 core::RigidBody rb=core::RigidBody::setup_particle(prb,rbps);
 rb->set_name(name);
 // Check orientation of x-axis and topology
 double bb = (core::RigidMember(rbps[0]).get_internal_coordinates())[0];
 double ee = (core::RigidMember(rbps[nres-1]).get_internal_coordinates())[0];
 if (ee-bb<0.0){
  for(unsigned int k=0;k<rbps.size();++k){
   algebra::Vector3D coord=
   core::RigidMember(rbps[k]).get_internal_coordinates();
   algebra::Rotation3D rot=
    algebra::get_rotation_about_axis(algebra::Vector3D(0,0,1),IMP::PI);
   algebra::Transformation3D tr=
    algebra::Transformation3D(rot,algebra::Vector3D(0,0,0));
   core::RigidMember(rbps[k]).set_internal_coordinates
    (tr.get_transformed(coord));
  }
 }
 rb.set_reference_frame(algebra::ReferenceFrame3D(algebra::Transformation3D
      (algebra::get_rotation_about_axis(algebra::Vector3D(0,1,0),-IMP::PI/2.0),
       x0)));
 return protein;
}


atom::Molecule create_merged_protein
(Model *m,std::string name,atom::Molecules proteins,
 int copy,double kappa,double dist)
{
 IMP_NEW(Particle,p,(m));
 atom::Molecule h=atom::Molecule::setup_particle(p);
 h->set_name(name);
 if (copy==0 && dist>=0.0){
  for(int j=0;j<proteins.size()-1;++j){
   add_internal_restraint(m,name,proteins[j],proteins[j+1],kappa,dist);
  }
 }
 for(int j=0;j<proteins.size();++j){
  Particles ps=atom::get_leaves(proteins[j]);
  for(int i=0;i<ps.size();++i){
   proteins[j].remove_child(atom::Domain(ps[i]));
   h.add_child(atom::Domain(ps[i]));
  }
 }
 return h;
}

IMPMEMBRANE_END_NAMESPACE
