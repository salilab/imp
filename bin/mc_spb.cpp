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
atom::Hierarchies all_mol=
 create_hierarchies(m,mydata.num_cells);

// List of particles for layer restraint
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));

// List of Movers for MC
core::Movers mvs;

//
// PROTEIN REPRESENTATION
//
// from here, it should be move to spb_representation

// atom::Hierarchies all_mol=
//  create_representation(m,mydata,CP_ps,IL2_ps,mvs);

for(int i=0;i<mydata.num_cells;++i){

 algebra::Vector3D CP_x0=mydata.CP_centers[i];
 algebra::Vector3D IL2_x0=mydata.IL2_centers[i];
 double ddz=mydata.CP_thickness/2.0+mydata.CP_IL2_gap/2.0;
 algebra::Vector3D CC_x0=algebra::Vector3D(CP_x0[0],CP_x0[1],ddz);

 for(int j=0;j<mydata.num_copies;++j){
 //for(int j=0;j<1;++j){

  //Spc42p_n, 2 copies, 1 bead
   atom::Molecule Spc42p_n_0=
   create_protein(m,"Spc42p_n",7,1,
                    display::Color(175./255.,238./255.,238./255.),
                    i,mydata.kappa,CP_x0);
  atom::Molecule Spc42p_n_1=
   create_protein(m,"Spc42p_n",7,1,
                    display::Color(175./255.,238./255.,238./255.),
                    i,mydata.kappa,CP_x0);
  //all_mol[i].add_child(Spc42p_n_0);
  //all_mol[i].add_child(Spc42p_n_1);
  if(i==0){
   Particles ps_Spc42p_n_0=atom::get_leaves(Spc42p_n_0);
   Particles ps_Spc42p_n_1=atom::get_leaves(Spc42p_n_1);
   CP_ps->add_particles(ps_Spc42p_n_0);
   CP_ps->add_particles(ps_Spc42p_n_1);
   for(unsigned int k=0;k<ps_Spc42p_n_0.size();++k){
     Particles pps;
     pps.push_back(ps_Spc42p_n_0[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
   }
   for(unsigned int k=0;k<ps_Spc42p_n_1.size();++k){
     Particles pps;
     pps.push_back(ps_Spc42p_n_1[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
   }
  }
/*
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
  all_mol[i].add_child(Spc29p);

  if(i==0){
   Particles ps_Spc29p=atom::get_leaves(Spc29p);
   CP_ps->add_particles(ps_Spc29p);
   IMP_NEW(membrane::PbcBoxedMover,mv,
          (ps_Spc29p[0],ps_Spc29p,mydata.MC.dx,mydata.CP_centers,mydata.trs));
   mvs.push_back(mv);
   for(unsigned int k=1;k<ps_Spc29p.size();++k){
    Particles pps;
    pps.push_back(ps_Spc29p[k]);
    IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
    mvs.push_back(bmv);
   }
  }

  //Spc110p_c, 3 beads
  atom::Molecule Spc110p_c=
   create_protein(m,"Spc110p_c",26,3,
                    display::Color(255./255.,0.,0.),
                    i,mydata.kappa,CP_x0,627+164);
  all_mol[i].add_child(Spc110p_c);
  if(i==0){
   Particles ps_Spc110p_c=atom::get_leaves(Spc110p_c);
   CP_ps->add_particles(ps_Spc110p_c);
   IMP_NEW(membrane::PbcBoxedMover,mv,
          (ps_Spc110p_c[0],ps_Spc110p_c,mydata.MC.dx,
          mydata.CP_centers,mydata.trs));
   mvs.push_back(mv);
   for(unsigned int k=1;k<ps_Spc110p_c.size();++k){
    Particles pps;
    pps.push_back(ps_Spc110p_c[k]);
    IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
    mvs.push_back(bmv);
   }
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
   all_mol[i].add_child(Cmd1p);
   if(i==0){
    Particles ps_Cmd1p=atom::get_leaves(Cmd1p);
    CP_ps->add_particles(ps_Cmd1p);
    IMP_NEW(membrane::PbcBoxedMover,mv,
          (ps_Cmd1p[0],ps_Cmd1p,mydata.MC.dx,mydata.CP_centers,mydata.trs));
    mvs.push_back(mv);
    for(unsigned int k=1;k<ps_Cmd1p.size();++k){
     Particles pps;
     pps.push_back(ps_Cmd1p[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
    }
   }
  }else{
   atom::Molecule Cmd1p=
    create_protein(m,"Cmd1p","3CLN.pdb",30,
                    display::Color(255./255.,255./255.,0.),
                    i,CP_x0,5);
   all_mol[i].add_child(Cmd1p);
   if(i==0){
    Particles ps_Cmd1p=atom::get_leaves(Cmd1p);
    Particles fake;
    CP_ps->add_particles(ps_Cmd1p);
    core::RigidBody prb=core::RigidMember(ps_Cmd1p[0]).get_rigid_body();
    IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
     (prb,fake,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs));
    mvs.push_back(rbmv);
   }
  }

// IL2
 //Cnm67p_c, 2 beads
 if(!mydata.use_structure){
  atom::Molecule Cnm67p_c=
   create_protein(m,"Cnm67p_c",15,2,
                    display::Color(50./255.,205./255.,50./255.),
                    i,mydata.kappa,IL2_x0,272+179);
  all_mol[i].add_child(Cnm67p_c);
  if(i==0){
    Particles ps_Cnm67p_c=atom::get_leaves(Cnm67p_c);
    IL2_ps->add_particles(ps_Cnm67p_c);
    IMP_NEW(membrane::PbcBoxedMover,mv,
     (ps_Cnm67p_c[0],ps_Cnm67p_c,mydata.MC.dx,mydata.CP_centers,mydata.trs));
    mvs.push_back(mv);
    for(unsigned int k=1;k<ps_Cnm67p_c.size();++k){
     Particles pps;
     pps.push_back(ps_Cnm67p_c[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
    }
  }
 } else {
  atom::Molecule Cnm67p_c=
   create_protein(m,"Cnm67p_c","3OA7.pdb",30,
                    display::Color(50./255.,205./255.,50./255.),
                    i,IL2_x0,429);
  all_mol[i].add_child(Cnm67p_c);
  if(i==0){
   Particles ps_Cnm67p_c=atom::get_leaves(Cnm67p_c);
   Particles fake;
   IL2_ps->add_particles(ps_Cnm67p_c);
   core::RigidBody prb=core::RigidMember(ps_Cnm67p_c[0]).get_rigid_body();
   IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
     (prb,fake,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs));
   mvs.push_back(rbmv);
  }
 }
*/
//Spc42p_c, 2 copies, 2 beads
   atom::Molecule Spc42p_c_0=
   create_protein(m,"Spc42p_c",13,2,
                    display::Color(175./255.,218./255.,238./255.),
                    i,mydata.kappa,IL2_x0,139);
  atom::Molecule Spc42p_c_1=
   create_protein(m,"Spc42p_c",13,2,
                    display::Color(175./255.,218./255.,238./255.),
                    i,mydata.kappa,IL2_x0,139);
  //all_mol[i].add_child(Spc42p_c_0);
  //all_mol[i].add_child(Spc42p_c_1);
  if(i==0){
   Particles ps_Spc42p_c_0=atom::get_leaves(Spc42p_c_0);
   Particles ps_Spc42p_c_1=atom::get_leaves(Spc42p_c_1);
   IL2_ps->add_particles(ps_Spc42p_c_0);
   IL2_ps->add_particles(ps_Spc42p_c_1);
   for(unsigned int k=0;k<ps_Spc42p_c_0.size();++k){
     Particles pps;
     pps.push_back(ps_Spc42p_c_0[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
   }
   for(unsigned int k=0;k<ps_Spc42p_c_1.size();++k){
     Particles pps;
     pps.push_back(ps_Spc42p_c_1[k]);
     IMP_NEW(core::BallMover,bmv,(pps,mydata.MC.dx));
     mvs.push_back(bmv);
   }
  }

// Coiled-Coil
 atom::Molecules Spc42p_CC=
   create_coiled_coil(m,"Spc42_CC","2Q6Q_A.pdb", "2Q6Q_B.pdb",30,
                     display::Color(175./255.,208./255.,238./255.),
                     i, CC_x0,67);
 //all_mol[i].add_child(Spc42p_CC[0]);
 //all_mol[i].add_child(Spc42p_CC[1]);
 if(i==0){
  Particles ps_Spc42p_n_0=atom::get_leaves(Spc42p_n_0);
  Particles ps_Spc42p_c_0=atom::get_leaves(Spc42p_c_0);
  Particles ps_Spc42p_n_1=atom::get_leaves(Spc42p_n_1);
  Particles ps_Spc42p_c_1=atom::get_leaves(Spc42p_c_1);
  Particles ps_Spc42p;
  ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_n_0.begin(),ps_Spc42p_n_0.end());
  ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_c_0.begin(),ps_Spc42p_c_0.end());
  ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_n_1.begin(),ps_Spc42p_n_1.end());
  ps_Spc42p.insert(ps_Spc42p.end(),ps_Spc42p_c_1.begin(),ps_Spc42p_c_1.end());
  Particle *ps_tmp=atom::get_leaves(Spc42p_CC[0])[0];
  core::RigidBody prb=core::RigidMember(ps_tmp).get_rigid_body();
  IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
     (prb,ps_Spc42p,mydata.MC.dx,mydata.MC.dang,mydata.CP_centers,mydata.trs));
  mvs.push_back(rbmv);
 }
// now create the merge
 atom::Molecules Spc42p_0_all,Spc42p_1_all;
 Spc42p_0_all.push_back(Spc42p_n_0);
 Spc42p_0_all.push_back(Spc42p_CC[0]);
 Spc42p_0_all.push_back(Spc42p_c_0);
 Spc42p_1_all.push_back(Spc42p_n_1);
 Spc42p_1_all.push_back(Spc42p_CC[1]);
 Spc42p_1_all.push_back(Spc42p_c_1);

 atom::Molecule Spc42p_0=
    create_merged_protein(m,"Spc42p",Spc42p_0_all,i,mydata.kappa,0.0);
 atom::Molecule Spc42p_1=
    create_merged_protein(m,"Spc42p",Spc42p_1_all,i,mydata.kappa,0.0);
 all_mol[i].add_child(Spc42p_0);
 all_mol[i].add_child(Spc42p_1);
 }
}

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
/*
//
// FRET
//
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
*/
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
