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
 display::Color colore,int copy,double kappa,int start_residue,int length)
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
  d.set_coordinates(algebra::Vector3D(0.1,0.1,0.1));
  d.set_coordinates_are_optimized(true);
  atom::Mass mm=atom::Mass::setup_particle(pp,ms);
  display::Colored cc=display::Colored::setup_particle(pp,colore);
  protein.add_child(dom);
 }
 if(nbeads>1 && copy==0){
  atom::Selections ss=atom::Selections();
  atom::HierarchiesTemp hs=protein.get_children();
  for(unsigned int i=0;i<hs.size();++i){
   ss.push_back(atom::Selection(hs[i]));
  }
  Restraint *con=atom::create_connectivity_restraint(ss,kappa);
  con->set_name("Connectivity Restraint for "+name);
  m->add_restraint(con);
 }
 return protein;
}

atom::Molecule create_merged_protein
(Model *m,std::string name,atom::Molecule protein_a,
atom::Molecule protein_b,int copy,double kappa,double dist)
{
 IMP_NEW(Particle,p,(m));
 atom::Molecule h=atom::Molecule::setup_particle(p);
 h->set_name(name);
 if (copy==0 and dist >=0.0){
   add_internal_restraint(m,name,protein_a,protein_b,kappa,dist);
 }
 ParticlesTemp psa=protein_a.get_leaves();
 for(int i=0;i<psa.size();++i){
  protein_a.remove_child(atom::Domain(psa[i]));
  h.add_child(atom::Domain(psa[i]));
 }
 ParticlesTemp psb=protein_b.get_leaves();
 for(int i=0;i<psb.size();++i){
  protein_b.remove_child(atom::Domain(psb[i]));
  h.add_child(atom::Domain(psb[i]));
 }
 return h;
}

IMPMEMBRANE_END_NAMESPACE
