/**
 *  \file spb_reload_minimize.cpp
 *  \brief SPB Minimizer
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <IMP/rmf.h>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{

// parsing input
std::cout << "Parsing input file" << std::endl;
SPBParameters mydata=get_SPBParameters("config.ini","0");

// create a new model
IMP_NEW(Model,m,());

// List of particles for layer restraint
IMP_NEW(container::ListSingletonContainer,bCP_ps,(m));
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));
// List of Movers for MC, not used here
core::Movers mvs;
//
// PROTEIN REPRESENTATION
//
std::cout << "Creating representation" << std::endl;
atom::Hierarchies all_mol=
 create_representation(m,mydata,bCP_ps,CP_ps,IL2_ps,mvs);
atom::Hierarchies hhs;
for(unsigned int i=0;i<all_mol.size();++i){
 atom::Hierarchies hs=all_mol[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {hhs.push_back(hs[j]);}
}
//
// READ LABEL FILE
//
std::ifstream labelfile;
std::string label;
std::vector<std::string> labels;
char* name=(char*)malloc( sizeof( char ) *(mydata.label.length() +1) );
strcpy(name, mydata.label.c_str());
labelfile.open(name);
while (labelfile >> label){labels.push_back(label);}
labelfile.close();
// assigment file
std::ofstream assfile;
assfile.open("assignment.dat");
//
core::XYZs cluster_ps;
for(unsigned int j=0;j<labels.size();++j){
 for(unsigned int i=0;i<hhs.size();++i){
  Particles ps=atom::get_leaves(hhs[i]);
  for(unsigned int k=0;k<ps.size();++k){
   if(atom::Domain(ps[k])->get_name()==labels[j]){
    cluster_ps.push_back(core::XYZ(ps[k]));
    assfile << j+1 << "\n";
   }
  }
 }
}
assfile.close();
//
// PREPARE INPUT/OUTPUT
//
std::ofstream trajfile, logscore;
trajfile.open("traj.xyz");
logscore.open("log.score");
//
RMF::FileHandle rh=RMF::open_rmf_file(mydata.trajfile);
rmf::set_hierarchies(rh, hhs);

// getting key for score
RMF::Category my_kc= rh.get_category("my data");
RMF::FloatKey my_key=rh.get_float_key(my_kc,"my score");
// number of frames
unsigned int nframes=rmf::get_number_of_frames(rh,hhs[0]);

for(unsigned int imc=0;imc<nframes;++imc){
 for(unsigned int i=0;i<hhs.size();++i){
  rmf::load_frame(rh,imc,hhs[i]);
 }
 double myscore = (rh.get_root_node()).get_value(my_key,imc);
// write score to file
 logscore << imc+1 << " " << myscore << "\n";
// write configuration to file
 trajfile << "XYZ " << imc+1 << "\n";
 for(unsigned int i=0;i<cluster_ps.size();++i){
  algebra::Vector3D xx=cluster_ps[i].get_coordinates();
  trajfile << xx[0] << " " << xx[1] << " " << xx[2] << "\n";
 }
}

return 0;
}
