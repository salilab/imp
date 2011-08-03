/**
 *  \file mc_spb.cpp
 *  \brief SPB MonteCarlo
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <IMP/rmf.h>
#include <IMP/display.h>
#include <boost/scoped_array.hpp>
#include <time.h>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{
// log file
std::ofstream logfile;
logfile.open("log");

// create a new model
IMP_NEW(Model,m,());

// parsing input
std::cout << "Parsing input file" << std::endl;
SPBParameters mydata=get_SPBParameters("config.ini");


std::cout << "Creating representation" << std::endl;
atom::Hierarchies h_CP_IL2=
 create_hierarchies(m,mydata.num_cells,"Central and IL2 Plaque");
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));
//
// PROTEIN REPRESENTATION
//

for(int i=0;i<mydata.num_cells;++i){

 algebra::Vector3D CP_x0=mydata.CP_centers[i];
 algebra::Vector3D IL2_x0=mydata.IL2_centers[i];
 //for(int j=0;j<mydata.num_copies;++j){
 for(int j=0;j<1;++j){

  //Spc42p_n, 2 copies, 1 bead
   atom::Molecule Spc42p_n_0=
   create_protein(m,"Spc42p_n",7,1,
                    display::Color(175./255.,238./255.,238./255.),
                    i,mydata.kappa,CP_x0);
  atom::Molecule Spc42p_n_1=
   create_protein(m,"Spc42p_n",7,1,
                    display::Color(175./255.,238./255.,238./255.),
                    i,mydata.kappa,CP_x0);
  //h_CP_IL2[i].add_child(Spc42p_n_0);
  //h_CP_IL2[i].add_child(Spc42p_n_1);
  if(i==0){
   CP_ps->add_particles(atom::get_leaves(Spc42p_n_0));
   CP_ps->add_particles(atom::get_leaves(Spc42p_n_1));
  }

  //Spc29p, 2 beads for N, 2 beads for C
  atom::Molecules Spc29p_all;
  atom::Molecule Spc29p_n=
   create_protein(m,"Spc29p_n",14.5,2,
                    display::Color(255./255.,165./255.,0.),
                    i,mydata.kappa,CP_x0);
  Spc29p_all.push_back(Spc29p_n);
  atom::Molecule Spc29p_c=
   create_protein(m,"Spc29p_c",14.5,2,
                    display::Color(255./255.,140./255.,0.),
                    i,mydata.kappa,CP_x0,132);
  Spc29p_all.push_back(Spc29p_c);
  atom::Molecule Spc29p=
   create_merged_protein(m,"Spc29p",Spc29p_all,i,mydata.kappa,0.0);
  h_CP_IL2[i].add_child(Spc29p);
  if(i==0){
   CP_ps->add_particles(atom::get_leaves(Spc29p));
  }

  //Spc110p_c, 3 beads
  atom::Molecule Spc110p_c=
   create_protein(m,"Spc110p_c",26,3,
                    display::Color(255./255.,0.,0.),
                    i,mydata.kappa,CP_x0,627+164);
  h_CP_IL2[i].add_child(Spc110p_c);
  if(i==0){
   CP_ps->add_particles(atom::get_leaves(Spc110p_c));
  }

 //Cmd1p, 1 bead for N, 1 bead for C
  if(!mydata.use_structure){
   atom::Molecules Cmd1p_all;
   atom::Molecule Cmd1p_n=
    create_protein(m,"Cmd1p_n",8,1,
                    display::Color(255./255.,255./255.,0.),
                    i,mydata.kappa,CP_x0);
   Cmd1p_all.push_back(Cmd1p_n);
   atom::Molecule Cmd1p_c=
    create_protein(m,"Cmd1p_c",8,1,
                    display::Color(255./255.,215./255.,0.),
                    i,mydata.kappa,CP_x0,80);
   Cmd1p_all.push_back(Cmd1p_c);
   atom::Molecule Cmd1p=
    create_merged_protein(m,"Cmd1p",Cmd1p_all,i,mydata.kappa,0.0);
   h_CP_IL2[i].add_child(Cmd1p);
   if(i==0){
    CP_ps->add_particles(atom::get_leaves(Cmd1p));
   }
  }else{
   atom::Molecule Cmd1p=
    create_protein(m,"Cmd1p","3CLN.pdb",30,
                    display::Color(255./255.,255./255.,0.),
                    i,CP_x0,5);
   h_CP_IL2[i].add_child(Cmd1p);
   if(i==0){
    CP_ps->add_particles(atom::get_leaves(Cmd1p));
   }
  }

// IL2
 //Cnm67p_c, 2 beads
 if(!mydata.use_structure){
  atom::Molecule Cnm67p_c=
   create_protein(m,"Cnm67p_c",15,2,
                    display::Color(50./255.,205./255.,50./255.),
                    i,mydata.kappa,IL2_x0,272+179);
  h_CP_IL2[i].add_child(Cnm67p_c);
  if(i==0){
   IL2_ps->add_particles(atom::get_leaves(Cnm67p_c));
  }
 } else {
  atom::Molecule Cnm67p_c=
   create_protein(m,"Cnm67p_c","3OA7.pdb",30,
                    display::Color(50./255.,205./255.,50./255.),
                    i,IL2_x0,429);
  h_CP_IL2[i].add_child(Cnm67p_c);
  if(i==0){
   IL2_ps->add_particles(atom::get_leaves(Cnm67p_c));
  }
 }

//Spc42p_c, 2 copies, 2 beads
   atom::Molecule Spc42p_c_0=
   create_protein(m,"Spc42p_c",13,2,
                    display::Color(175./255.,218./255.,238./255.),
                    i,mydata.kappa,IL2_x0,77+60);
  atom::Molecule Spc42p_c_1=
   create_protein(m,"Spc42p_c",13,2,
                    display::Color(175./255.,218./255.,238./255.),
                    i,mydata.kappa,IL2_x0,77+60);
  //h_CP_IL2[i].add_child(Spc42p_c_0);
  //h_CP_IL2[i].add_child(Spc42p_c_1);
  if(i==0){
   IL2_ps->add_particles(atom::get_leaves(Spc42p_c_0));
   IL2_ps->add_particles(atom::get_leaves(Spc42p_c_1));
  }

// now create the merge
 atom::Molecules Spc42p_0_all,Spc42p_1_all;
 Spc42p_0_all.push_back(Spc42p_n_0);
 Spc42p_0_all.push_back(Spc42p_c_0);
 Spc42p_1_all.push_back(Spc42p_n_1);
 Spc42p_1_all.push_back(Spc42p_c_1);
 atom::Molecule Spc42p_0=
    create_merged_protein(m,"Spc42p",Spc42p_0_all,i,mydata.kappa,108.0);
 atom::Molecule Spc42p_1=
    create_merged_protein(m,"Spc42p",Spc42p_1_all,i,mydata.kappa,108.0);
 h_CP_IL2[i].add_child(Spc42p_0);
 h_CP_IL2[i].add_child(Spc42p_1);
 }
}

// trajectory file
std::string trajname="traj.rmf";
rmf::RootHandle rh = rmf::RootHandle(trajname,true);
for(unsigned int i=0;i<h_CP_IL2.size();++i){
 atom::Hierarchies hs=h_CP_IL2[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {rmf::add_hierarchy(rh, hs[j]);}
}

//
// CREATING RESTRAINTS
std::cout << "Creating restraints" << std::endl;
//
// Excluded volume
//
add_SPBexcluded_volume(m,h_CP_IL2,mydata.kappa);
//
// Symmetry
//
add_symmetry_restraint(m,h_CP_IL2,mydata.trs);
//
// Layer restraint
//
add_layer_restraint(m, CP_ps,
 FloatRange(-mydata.CP_thickness/2.0,mydata.CP_thickness/2.0),
 mydata.kappa);
double dz=mydata.IL2_centers[0][2];
add_layer_restraint(m, IL2_ps,
 FloatRange(-mydata.IL2_thickness/2.0+dz,mydata.IL2_thickness/2.0+dz),
 mydata.kappa);
//
// FRET
//
// intra-CP
/*
add_fret_restraint(m,h_CP_IL2, "Spc29p",   "C",
                     h_CP_IL2, "Cmd1p",     "C", 1.69, mydata.kappa);
add_fret_restraint(m,h_CP_IL2, "Spc29p",   "N",
                     h_CP_IL2, "Cmd1p",     "C", 1.75, mydata.kappa);
add_fret_restraint(m,h_CP_IL2, "Spc29p",   "C",
                     h_CP_IL2, "Spc110p_c", "C", 1.37, mydata.kappa);
add_fret_restraint(m,h_CP_IL2, "Spc29p",   "C",
                     h_CP_IL2, "Spc42p_n",  "N", 2.05, mydata.kappa);
add_fret_restraint(m,h_CP_IL2, "Cmd1p",    "C",
                     h_CP_IL2, "Spc42p_n",  "N", 2.07, mydata.kappa);
add_fret_restraint(m,h_CP_IL2, "Cmd1p",    "C",
                     h_CP_IL2, "Spc110p_c", "C", 2.15, mydata.kappa);
add_fret_restraint(m,h_CP_IL2, "Spc42p_n", "N",
                     h_CP_IL2, "Spc110p_c", "C", 2.02, mydata.kappa);
//
// TWO-HYBRID SCREENING
//
// CP
add_y2h_restraint(m,h_CP_IL2, "Cmd1p",      "ALL",
                    h_CP_IL2, "Spc110p_c", IntRange(900,944), mydata.kappa);
add_y2h_restraint(m,h_CP_IL2, "Spc42p_n",     "N",
                    h_CP_IL2, "Spc110p_c",    "C", mydata.kappa);
add_y2h_restraint(m,h_CP_IL2, "Spc29p",       "ALL",
                    h_CP_IL2, "Spc110p_c", IntRange(811,944), mydata.kappa);
add_y2h_restraint(m,h_CP_IL2, "Spc110p_c",    "C",
                    h_CP_IL2, "Spc110p_c",    "C", mydata.kappa);
add_y2h_restraint(m,h_CP_IL2, "Spc42p_n", IntRange(1,138),
                    h_CP_IL2, "Spc29p",       "ALL", mydata.kappa);
add_y2h_restraint(m,h_CP_IL2, "Spc42p_n", IntRange(1,138),
                    h_CP_IL2, "Spc42p_n", IntRange(1,138), mydata.kappa);
*/
std::cout << "Setup sampler" << std::endl;
atom::Hierarchies hs;
hs.push_back(h_CP_IL2[0]);
//hs.push_back(h_IL2[0]);
core::MonteCarlo* mc=setup_SPBMonteCarlo(m,hs,mydata.MC.tmin,mydata);

std::cout << "Sampling" << std::endl;
// Monte Carlo loop
for(int imc=0;imc<mydata.MC.nsteps;++imc)
{
// run optimizer
 mc->optimize(mydata.MC.nexc);

// print statistics
 double myscore=m->evaluate(false);
 logfile << imc << " " << myscore << " " <<
  mc->get_number_of_forward_steps() << "\n";

// save configuration to file
 if(imc%mydata.MC.nwrite==0){
  for(unsigned int i=0;i<h_CP_IL2.size();++i){
   atom::Hierarchies hs=h_CP_IL2[i].get_children();
   for(unsigned int j=0;j<hs.size();++j){
    rmf::save_frame(rh,imc/mydata.MC.nwrite,hs[j]);
   }
  }
 }

}

logfile.close();
return 0;
}
