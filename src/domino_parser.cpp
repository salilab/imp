/**
 *  \file domino_parse.cpp
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <boost/program_options.hpp>
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
 std::vector<std::string> res, inter, loop;
 bool add_dope, add_pack, use_volume;


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
 desc.add_options()("add_pack",    value< bool >(&add_pack),           "ciao");
 desc.add_options()("use_volume",  value< bool >(&use_volume),         "ciao");


 OPTION(double, diameter);
 OPTION(double, tilt);
 OPTION(double, swing);
 OPTION(double, rot);
 OPTION(double, x);
 OPTION(double, tiltmax);
 OPTION(double, swingmax);
 OPTION(double, rotmax);
 OPTION(double, zmax);
 OPTION(double, tmin);
 OPTION(double, tmax);
 OPTION(double, dx);
 OPTION(double, dang);
 OPTION(int,    number);
 OPTION(int,    nexc);
 OPTION(int,    nsteps);

 variables_map vm;
 store(parse_config_file(in.get_stream(), desc, false), vm);
 notify(vm);

 CHECK(double, diameter);
 CHECK(double, tilt);
 CHECK(double, swing);
 CHECK(double, rot);
 CHECK(double, x);
 CHECK(double, tiltmax);
 CHECK(double, swingmax);
 CHECK(double, rotmax);
 CHECK(double, zmax);
 CHECK(double, tmin);
 CHECK(double, tmax);
 CHECK(double, dx);
 CHECK(double, dang);
 CHECK(int,    number);
 CHECK(int,    nexc);
 CHECK(int,    nsteps);

 Parameters ret;

// General Parameters
 ret.diameter=diameter;
 ret.score_name=score_name;
 ret.ass_file=ass_file;
 ret.traj_file=traj_file;
 ret.add_dope=add_dope;
 ret.add_pack=add_pack;
 ret.use_volume=use_volume;

// MonteCarlo Parameters
 ret.MC.tmin=tmin;
 ret.MC.tmax=tmax;
 ret.MC.nexc=nexc;
 ret.MC.nsteps=nsteps;
 ret.MC.dx=dx;
 ret.MC.dang=dang;

// Grid Parameters
 ret.grid.tilt=radians(tilt);
 ret.grid.swing=radians(swing);
 ret.grid.rot=radians(rot);
 ret.grid.x=x;
 ret.grid.tiltmax=radians(tiltmax);
 ret.grid.swingmax=radians(swingmax);
 ret.grid.rotmax=radians(rotmax);
 ret.grid.xmax=diameter;
 ret.grid.zmax=zmax;

// HelixData
 ret.TM.num=number;

 std::vector<char> v(sequence.begin(), sequence.end());
 ret.TM.seq = v;

 if (vm.count("residues")){
  res = vm["residues"].as< std::vector<std::string> >();
  for(unsigned int i=0;i<res.size();++i){
   std::vector<std::string> strs;
   boost::split(strs, res[i], boost::is_any_of("\t "));
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
   boost::split(strs, inter[i], boost::is_any_of("\t "));
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
   boost::split(strs, loop[i], boost::is_any_of("\t "));
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
