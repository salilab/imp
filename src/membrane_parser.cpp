/**
 *  \file  membrane_parser.cpp
 *  \brief Membrane stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <IMP/base_types.h>
#include <IMP/membrane.h>
#include <string>

using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

#define OPTION(type, name)                                   \
 type name=-1;                                              \
 desc.add_options()(#name, value< type >(&name), #name);

#define CHECK(type, name)                                    \
 IMP_USAGE_CHECK(name >=0, #name << " is " << name);


Parameters get_parameters(TextInput in) {
#if BOOST_VERSION >= 104100
 using namespace boost::program_options;
 options_description desc;
 std::string score_name,ass_file,traj_file,sequence;
 std::vector<std::string> res,inter,loop;
 bool do_random,add_dope,use_volume,do_wte;


 desc.add_options()("score_name",  value< std::string >(&score_name),  "ciao");
 desc.add_options()("ass_file",    value< std::string >(&ass_file),    "ciao");
 desc.add_options()("traj_file",   value< std::string >(&traj_file),   "ciao");
 desc.add_options()("sequence",    value< std::string >(&sequence),    "ciao");
 desc.add_options()("residues",    value< std::vector<std::string> >(),"ciao");
 desc.add_options()("topology",    value< std::vector<double> >(),     "ciao");
 desc.add_options()("name",        value< std::vector<std::string> >(),"ciao");
 desc.add_options()("structure",   value< std::vector<std::string> >(),"ciao");
 desc.add_options()("interacting", value< std::vector<std::string> >(),"ciao");
 desc.add_options()("loop",        value< std::vector<std::string> >(),"ciao");
 desc.add_options()("add_dope",    value< bool >(&add_dope),           "ciao");
 desc.add_options()("use_volume",  value< bool >(&use_volume),         "ciao");
 desc.add_options()("do_wte",      value< bool >(&do_wte),             "ciao");
 desc.add_options()("do_random",   value< bool >(&do_random),          "ciao");


 OPTION(double, grid_dtilt);
 OPTION(double, grid_dswing);
 OPTION(double, grid_drot);
 OPTION(double, grid_dx);
 OPTION(double, grid_tiltmax);
 OPTION(double, grid_swingmax);
 OPTION(double, grid_rotmax);
 OPTION(double, grid_zmax);
 OPTION(double, mc_tmin);
 OPTION(double, mc_tmax);
 OPTION(double, mc_dx);
 OPTION(double, mc_dang);
 OPTION(double, diameter);
 OPTION(double, tiltrange);
 OPTION(double, zetarange);
 OPTION(double, cm_dist);
 OPTION(double, d0_inter);
 OPTION(double, k_depth)
 OPTION(double, k_tilt)
 OPTION(double, k_volume)
 OPTION(double, k_pack)
 OPTION(double, k_diameter)
 OPTION(double, k_endtoend)
 OPTION(double, k_cmdist)
 OPTION(double, k_inter)
 OPTION(double, wte_w0);
 OPTION(double, wte_sigma);
 OPTION(double, wte_gamma);
 OPTION(double, wte_emin);
 OPTION(double, wte_emax);
 OPTION(int,    number);
 OPTION(int,    mc_nexc);
 OPTION(int,    mc_nsteps);
 OPTION(int,    mc_nhot);
 OPTION(int,    mc_nwrite);

 variables_map vm;
 store(parse_config_file(in.get_stream(), desc, false), vm);
 notify(vm);

 Parameters ret;

// General Parameters
 ret.ass_file=ass_file;
 ret.traj_file=traj_file;
 ret.do_random=do_random;

// MonteCarlo Parameters
 ret.MC.tmin=mc_tmin;
 ret.MC.tmax=mc_tmax;
 ret.MC.nexc=mc_nexc;
 ret.MC.nsteps=mc_nsteps;
 ret.MC.nhot=mc_nhot;
 ret.MC.nwrite=mc_nwrite;
 ret.MC.dx=mc_dx;
 ret.MC.dang=mc_dang;
 ret.MC.do_wte=do_wte;
 ret.MC.wte_w0=wte_w0;
 ret.MC.wte_sigma=wte_sigma;
 ret.MC.wte_gamma=wte_gamma;
 ret.MC.wte_emin=wte_emin;
 ret.MC.wte_emax=wte_emax;

// Restraints Parameters
 ret.RST.diameter=diameter;
 ret.RST.score_name=score_name;
 ret.RST.add_dope=add_dope;
 ret.RST.use_volume=use_volume;
 ret.RST.tilt=radians(tiltrange);
 ret.RST.zeta=zetarange;
 ret.RST.cm_dist=cm_dist;
 ret.RST.d0_inter=d0_inter;
 // kappa
 ret.RST.k_depth=k_depth;
 ret.RST.k_tilt=k_tilt;
 ret.RST.k_volume=k_volume;
 ret.RST.k_pack=k_pack;
 ret.RST.k_diameter=k_diameter;
 ret.RST.k_endtoend=k_endtoend;
 ret.RST.k_cmdist=k_cmdist;
 ret.RST.k_inter=k_inter;

// Grid Parameters
 ret.grid.tilt=radians(grid_dtilt);
 ret.grid.swing=radians(grid_dswing);
 ret.grid.rot=radians(grid_drot);
 ret.grid.x=grid_dx;
 ret.grid.tiltmax=radians(grid_tiltmax);
 ret.grid.swingmax=radians(grid_swingmax);
 ret.grid.rotmax=radians(grid_rotmax);
 ret.grid.xmax=diameter;
 ret.grid.zmax=grid_zmax;

// HelixData
 ret.TM.num=number;

 std::vector<char> v(sequence.begin(), sequence.end());
 ret.TM.seq = v;

 if (vm.count("residues")){
  res = vm["residues"].as< std::vector<std::string> >();
  for(unsigned int i=0;i<res.size();++i){
   std::vector<std::string> strs;
   boost::split(strs, res[i],  boost::is_any_of(","));
   ret.TM.resid.push_back
   (std::pair<int,int> (atoi(strs[0].c_str()),atoi(strs[1].c_str())));
  }
 }

 if (vm.count("topology")){
  ret.TM.topo = vm["topology"].as< std::vector<double> >();
 }

 if (vm.count("name")){
  ret.TM.name = vm["name"].as< std::vector<std::string> >();
 }

 if (vm.count("structure")){
  ret.TM.struct_file = vm["structure"].as< std::vector<std::string> >();
 }

 if (vm.count("interacting")){
  inter = vm["interacting"].as< std::vector<std::string> >();
  for(unsigned int i=0;i<inter.size();++i){
   std::vector<std::string> strs;
   int pos1,pos2;
   boost::split(strs, inter[i],  boost::is_any_of(","));
   pos1 = std::find(ret.TM.name.begin(),ret.TM.name.end(),strs[0]) -
   ret.TM.name.begin();
   pos2 = std::find(ret.TM.name.begin(),ret.TM.name.end(),strs[1]) -
   ret.TM.name.begin();
   ret.TM.inter.push_back(std::pair<int,int> (pos1,pos2));
  }
 }

if (vm.count("loop")){
  loop = vm["loop"].as< std::vector<std::string> >();
  for(unsigned int i=0;i<loop.size();++i){
   std::vector<std::string> strs;
   int pos1,pos2;
   boost::split(strs, loop[i],  boost::is_any_of(","));
   pos1 = std::find(ret.TM.name.begin(),ret.TM.name.end(),strs[0]) -
   ret.TM.name.begin();
   pos2 = std::find(ret.TM.name.begin(),ret.TM.name.end(),strs[1]) -
   ret.TM.name.begin();
   ret.TM.loop.push_back(std::pair<int,int> (pos1,pos2));
  }
 }

if(ret.TM.num!=ret.TM.name.size())
IMP_FAILURE("Check the name list!");
if(ret.TM.num!=ret.TM.topo.size())
IMP_FAILURE("Check the topology list!");
if(ret.TM.num!=ret.TM.resid.size())
IMP_FAILURE("Check the residues list!");
if(ret.TM.num!=ret.TM.struct_file.size())
IMP_FAILURE("Check the structure list!");

 return ret;
#else
 IMP_FAILURE("Need newer boost");
#endif
}

IMPMEMBRANE_END_NAMESPACE
