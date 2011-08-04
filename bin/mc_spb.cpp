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

//
// PROTEIN REPRESENTATION
//
std::cout << "Creating representation" << std::endl;
// List of particles for layer restraint
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));
// List of Movers for MC
core::Movers mvs;

atom::Hierarchies all_mol=
 create_representation(m,mydata,CP_ps,IL2_ps,mvs);

//
// trajectory file
std::string trajname="traj.rmf";
rmf::RootHandle rh = rmf::RootHandle(trajname,true);
for(unsigned int i=0;i<all_mol.size();++i){
 atom::Hierarchies hs=all_mol[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {rmf::add_hierarchy(rh, hs[j]);}
}

//
// CREATING RESTRAINTS
std::cout << "Creating restraints" << std::endl;
//
// Excluded volume
//
add_SPBexcluded_volume(m,all_mol,mydata.kappa);
//
// Symmetry
//
add_symmetry_restraint(m,all_mol,mydata.trs);
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
if(mydata.add_fret){
// intra-CP
 add_fret_restraint(m,all_mol, "Spc29p",    "C",
                      all_mol, "Cmd1p",     "C", 1.69, mydata.kappa);
 add_fret_restraint(m,all_mol, "Spc29p",    "N",
                      all_mol, "Cmd1p",     "C", 1.75, mydata.kappa);
 add_fret_restraint(m,all_mol, "Spc29p",    "C",
                      all_mol, "Spc110p_c", "C", 1.37, mydata.kappa);
 add_fret_restraint(m,all_mol, "Spc29p",    "C",
                      all_mol, "Spc42p",    "N", 2.05, mydata.kappa);
 add_fret_restraint(m,all_mol, "Cmd1p",     "C",
                      all_mol, "Spc42p",    "N", 2.07, mydata.kappa);
 add_fret_restraint(m,all_mol, "Cmd1p",     "C",
                      all_mol, "Spc110p_c", "C", 2.15, mydata.kappa);
 add_fret_restraint(m,all_mol, "Spc42p",    "N",
                      all_mol, "Spc110p_c", "C", 2.02, mydata.kappa);
// inter CP-IL2
 add_fret_restraint(m,all_mol, "Spc42p",    "C",
                      all_mol, "Spc110p_c", "C", 1.07, mydata.kappa);
 add_fret_restraint(m,all_mol, "Cnm67p_c",  "C",
                      all_mol, "Cmd1p",     "C", 1.09, mydata.kappa);
 add_fret_restraint(m,all_mol, "Spc42p",    "C",
                      all_mol, "Cmd1p",     "C", 1.1,  mydata.kappa);
 add_fret_restraint(m,all_mol, "Cnm67p_c",  "C",
                      all_mol, "Spc29p",    "C", 1.1,  mydata.kappa);
 add_fret_restraint(m,all_mol, "Cnm67p_c",  "C",
                      all_mol, "Spc42p",    "N", 1.13, mydata.kappa);
 add_fret_restraint(m,all_mol, "Spc42p",    "C",
                      all_mol, "Spc29p",    "C", 1.17, mydata.kappa);
 add_fret_restraint(m,all_mol, "Spc42p",    "C",
                      all_mol, "Spc42p",    "N", 1.27, mydata.kappa);
// intra-IL2
 add_fret_restraint(m,all_mol, "Spc42p",    "C",
                      all_mol, "Cnm67p_c",  "C", 2.29, mydata.kappa);
}
if(mydata.add_y2h){
//
// TWO-HYBRID SCREENING
//
// CP
 add_y2h_restraint(m,all_mol, "Cmd1p",      "ALL",
                     all_mol, "Spc110p_c",  IntRange(900,944), mydata.kappa);
 add_y2h_restraint(m,all_mol, "Spc42p",     "N",
                     all_mol, "Spc110p_c",  "C", mydata.kappa);
 add_y2h_restraint(m,all_mol, "Spc29p",     "ALL",
                     all_mol, "Spc110p_c",  IntRange(811,944), mydata.kappa);
 add_y2h_restraint(m,all_mol, "Spc110p_c",  "C",
                     all_mol, "Spc110p_c",  "C", mydata.kappa);
 add_y2h_restraint(m,all_mol, "Spc42p",     IntRange(1,138),
                     all_mol, "Spc29p",     "ALL", mydata.kappa);
 add_y2h_restraint(m,all_mol, "Spc42p",     IntRange(1,138),
                     all_mol, "Spc42p",     IntRange(1,138), mydata.kappa);
// IL2
 add_y2h_restraint(m,all_mol, "Cnm67p_c",   "C",
                     all_mol, "Spc42p",     "C", mydata.kappa);
}
//
//
std::cout << "Setup sampler" << std::endl;
core::MonteCarlo* mc=setup_SPBMonteCarlo(m,mvs,mydata.MC.tmin,mydata);

// hot steps
std::cout << "High temperature initialization" << std::endl;
mc->set_kt(mydata.MC.tmax);
mc->optimize(mydata.MC.nhot);
mc->set_kt(mydata.MC.tmin);

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
  for(unsigned int i=0;i<all_mol.size();++i){
   atom::Hierarchies hs=all_mol[i].get_children();
   for(unsigned int j=0;j<hs.size();++j){
    rmf::save_frame(rh,imc/mydata.MC.nwrite,hs[j]);
   }
  }
 }

}

logfile.close();
return 0;
}
