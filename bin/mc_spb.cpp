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
//
//h_CP: list of molecular hierarchies, containing
//proteins in the primitive cell  h_CP[0]
//proteins in the i-th      cell  h_CP[i]
atom::Hierarchies h_CP=create_hierarchies(m,mydata.num_cells,"Central Plaque");
//
// PROTEIN REPRESENTATION
//
//1) proteins in the Central Plaque
// Spc42p  (N)
// Spc29p  (N and C)
// Spc110p (C)
// Cmd1p   (N and C)
//

for(int i=0;i<mydata.num_cells;++i){
 for(int j=0;j<mydata.num_copies;++j){
  //Spc42p_n, 2 copies, 1 bead
  atom::Molecule Spc42p_n_0=
   create_protein(m,"Spc42p_n",7,1,
                    display::Color(175./255.,238./255.,238./255.),
                    i,mydata.kappa);
  atom::Molecule Spc42p_n_1=
   create_protein(m,"Spc42p_n",7,1,
                    display::Color(175./255.,238./255.,238./255.),
                    i,mydata.kappa);
  h_CP[i].add_child(Spc42p_n_0);
  h_CP[i].add_child(Spc42p_n_1);
  //Spc29p, 2 beads
  atom::Molecule Spc29p_n=
   create_protein(m,"Spc29p_n",14.5,1,
                    display::Color(255./255.,165./255.,0.),
                    i,mydata.kappa);
  atom::Molecule Spc29p_c=
   create_protein(m,"Spc29p_c",14.5,1,
                    display::Color(255./255.,140./255.,0.),
                    i,mydata.kappa,132);
  atom::Molecule Spc29p=
   create_merged_protein(m,"Spc29p",Spc29p_n,Spc29p_c,i,mydata.kappa,0.0);
  h_CP[i].add_child(Spc29p);
  //Spc110p_c, 3 beads
  atom::Molecule Spc110p_c=
   create_protein(m,"Spc110p_c",26,1,
                    display::Color(255./255.,0.,0.),
                    i,mydata.kappa,627+164);
  h_CP[i].add_child(Spc110p_c);
  //Cmd1p, 1 bead
  atom::Molecule Cmd1p_n=
   create_protein(m,"Cmd1p_n",8,1,
                    display::Color(255./255.,255./255.,0.),
                    i,mydata.kappa);
  atom::Molecule Cmd1p_c=
   create_protein(m,"Cmd1p_c",8,1,
                    display::Color(255./255.,215./255.,0.),
                    i,mydata.kappa, 80);
  atom::Molecule Cmd1p=
   create_merged_protein(m,"Cmd1p",Cmd1p_n,Cmd1p_c,i,mydata.kappa,0.0);
  h_CP[i].add_child(Cmd1p);
 }
}

// trajectory file
std::string trajname="traj.rmf";
rmf::RootHandle rh = rmf::RootHandle(trajname,true);
for(unsigned int i=0;i<h_CP.size();++i){
 atom::HierarchiesTemp hs=h_CP[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {rmf::add_hierarchy(rh, hs[j]);}
}
//

// CREATING RESTRAINTS
std::cout << "Creating restraints" << std::endl;
//
// Excluded volume
//
add_SPBexcluded_volume(m,h_CP,mydata.kappa);
//
// Symmetry
//
add_symmetry_restraint(m,h_CP,mydata);
//
// Surface restraint
//
add_surface_restraint(m, h_CP[0], "Cmd1p", "C",
                         mydata.CP_thickness/2.0, mydata.kappa);
add_surface_restraint(m, h_CP[0], "Cmd1p", "N",
                         mydata.CP_thickness/2.0, mydata.kappa);
add_surface_restraint(m, h_CP[0], "Spc110p_c", "C",
                         -mydata.CP_thickness/2.0, mydata.kappa);
//
// FRET
//
// intra-CP
add_fret_restraint(m,h_CP, "Spc29p",   "C",
                     h_CP, "Cmd1p",     "C", 1.69, mydata.kappa);
add_fret_restraint(m,h_CP, "Spc29p",   "N",
                     h_CP, "Cmd1p",     "C", 1.75, mydata.kappa);
add_fret_restraint(m,h_CP, "Spc29p",   "C",
                     h_CP, "Spc110p_c", "C", 1.37, mydata.kappa);
add_fret_restraint(m,h_CP, "Spc29p",   "C",
                     h_CP, "Spc42p_n",  "N", 2.05, mydata.kappa);
add_fret_restraint(m,h_CP, "Cmd1p",    "C",
                     h_CP, "Spc42p_n",  "N", 2.07, mydata.kappa);
add_fret_restraint(m,h_CP, "Cmd1p",    "C",
                     h_CP, "Spc110p_c", "C", 2.15, mydata.kappa);
add_fret_restraint(m,h_CP, "Spc42p_n", "N",
                     h_CP, "Spc110p_c", "C", 2.02, mydata.kappa);
//
// TWO-HYBRID SCREENING
//
// CP
add_y2h_restraint(m,h_CP, "Cmd1p",      "ALL",
                    h_CP, "Spc110p_c", IntRange(900,1020), mydata.kappa);
add_y2h_restraint(m,h_CP, "Spc42p_n",     "N",
                    h_CP, "Spc110p_c",    "C", mydata.kappa);
add_y2h_restraint(m,h_CP, "Spc29p",       "ALL",
                    h_CP, "Spc110p_c", IntRange(811,944), mydata.kappa);
add_y2h_restraint(m,h_CP, "Spc110p_c",    "C",
                    h_CP, "Spc110p_c",    "C", mydata.kappa);
add_y2h_restraint(m,h_CP, "Spc42p_n", IntRange(1,138),
                    h_CP, "Spc29p",       "ALL", mydata.kappa);
add_y2h_restraint(m,h_CP, "Spc42p_n", IntRange(1,138),
                    h_CP, "Spc42p_n", IntRange(1,138), mydata.kappa);


std::cout << "Setup sampler" << std::endl;
atom::Hierarchies hs;
hs.push_back(h_CP[0]);
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
  for(unsigned int i=0;i<h_CP.size();++i){
   atom::HierarchiesTemp hs=h_CP[i].get_children();
   for(unsigned int j=0;j<hs.size();++j){
    rmf::save_frame(rh,imc/mydata.MC.nwrite,hs[j]);
   }
  }
 }

}

logfile.close();
return 0;
}
