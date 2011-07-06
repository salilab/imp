/**
 *  \file spb.cpp
 *  \brief SPB in C++
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <boost/program_options.hpp>
#include <IMP/base_types.h>
#include <string>
#include <iostream>
using namespace IMP;

// various parameters
const double ds=40.0;
double       side=80.0;
const int    niter=3;
bool         do_statistics=true;
bool         do_random=true;
bool         do_save_ass=false;
const int    skip=100;
std::string  cell_type="hexagon";
int          num_cells;
int          num_copies;
double       error_bound;

algebra::Vector2Ds do_compress
(algebra::Vector2Ds points, double xf, double yf)
{
 algebra::Vector2Ds ret=algebra::Vector2Ds();
 for(int i=0;i<points.size();++i){
  ret.push_back(algebra::Vector2D(points[i][0]*xf,points[i][1]*yf));
 }
 return ret;
}

algebra::Vector2Ds do_shear
(algebra::Vector2Ds points, double bh, double h)
{
 algebra::Vector2Ds ret=algebra::Vector2Ds();
 double b=bh/h;
 for(int i=0;i<points.size();++i){
  ret.push_back(algebra::Vector2D(points[i][0]-b*points[i][1],points[i][1]));
 }
 return ret;
}

algebra::Vector3Ds grid_cell(double side,double ds,double z)
{
 algebra::BoundingBox2D bb=algebra::BoundingBox2D(algebra::Vector2D(0.0,0.0),
                                               algebra::Vector2D(side,side));
 algebra::Vector2Ds cur=algebra::get_grid_interior_cover_by_spacing(bb, ds);

 if(cell_type!="square"){
  algebra::Vector2Ds cur_comp=do_compress(cur,1.0,sqrt(3.0)/2.0);
  cur=do_shear(cur_comp,side/2.0,side*sqrt(3.0)/2.0);
 }

 algebra::Vector3Ds positions=algebra::Vector3Ds();
 algebra::Vector3Ds positions2=algebra::Vector3Ds();
 for(int i=0;i<cur.size();++i){
  positions.push_back(algebra::Vector3D(cur[i][0],cur[i][1],z));
 }
 if(cell_type=="hexagon"){
  algebra::Vector3D tra=algebra::Vector3D(0.0,0.0,0.0);
  for(int i=0;i<3;++i){
   algebra::Rotation3D rot=
   algebra::get_rotation_about_axis(algebra::Vector3D(0.0,0.0,1.0),
                                    (double)i * 2.0 * IMP::PI / 3.0);
   algebra::Transformation3D tr=algebra::Transformation3D(rot,tra);
   for(int j=0;j<positions.size();++j){
    positions2.push_back(tr.get_transformed(positions[j]));
   }
  }
 }
 //positions+=positions2;
 return positions;
}

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

atom::Molecule create_protein
(Model *m,std::string name,double mass,int nbeads,
int copy,int start_residue=-1,int length=-1)
{
 if(length==-1) {length=(int) mass*1000.0/110;}
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
  core::XYZR d=core::XYZR::setup_particle(pp);
  d.set_radius(rg);
  atom::Mass mm=atom::Mass::setup_particle(pp,ms);
  protein.add_child(dom);
 }
 if(nbeads>1 && copy==0){
  //con=IMP.atom.create_connectivity_restraint([IMP.atom.Selection(c) \
  //                             for c in protein.get_children()],1.0)
  //con.set_name("Connectivity Restraint for "+name)
  //model.add_restraint(con)
  //model.set_maximum_score(con, error_bound)
 }
 return protein;
}

core::DistancePairScore* get_pair_score(FloatRange dist)
{
 IMP_NEW(core::HarmonicWell,hw,(dist,1.0));
 IMP_NEW(core::DistancePairScore,ps,(hw));
 return ps.release();
}

void add_internal_restraint(Model *m,std::string name,
atom::Molecule protein_a,atom::Molecule protein_b,double dist)
{
 core::DistancePairScore* ps=get_pair_score(FloatRange(-500.0, dist));
 atom::Selection sa=atom::Selection(protein_a);
 atom::Selection sb=atom::Selection(protein_b);
 sa.set_terminus(atom::Selection::C);
 sa.set_terminus(atom::Selection::N);
 Particle*  pa=sa.get_selected_particles()[0];
 Particle*  pb=sb.get_selected_particles()[0];
 IMP_NEW(core::PairRestraint,r,(ps,ParticlePair(pa, pb)));
 r->set_name("IR " + name);
 m->set_maximum_score(r, error_bound);
 m->add_restraint(r);
}

atom::Molecule create_merged_protein
(Model *m,std::string name,atom::Molecule protein_a,
atom::Molecule protein_b,int copy,double dist=-1.0)
{
 IMP_NEW(Particle,p,(m));
 atom::Molecule h=atom::Molecule::setup_particle(p);
 h->set_name(name);
 if (copy==0 and dist >=0.0){
  add_internal_restraint(m,name,protein_a,protein_b,dist);
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

int main(int  , char **)
{

// cell dependent parameters
if(cell_type=="rhombus"){
 num_cells=21;
 num_copies=2;
 error_bound=1.45*pow(ds,2);
}else if(cell_type=="hexagon"){
 num_cells=7;
 num_copies=6;
 error_bound=1.45*pow(ds,2);
}else if(cell_type=="square"){
 num_cells=9;
 num_copies=6;
 side=sqrt(1.5*pow(side,2)*sqrt(3.0));
 error_bound=pow(ds,2);
}

// create a new model
IMP_NEW(Model,m,());

return 0;
}
