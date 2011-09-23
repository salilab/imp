/**
 *  \file spb_parser.cpp
 *  \brief SPB parser
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <boost/program_options.hpp>
#include <IMP/base_types.h>
#include <IMP/membrane.h>
#include <string>

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

#define OPTION(type, name)                                   \
 type name=-1;                                              \
 desc.add_options()(#name, value< type >(&name), #name);

SPBParameters get_SPBParameters(TextInput in, std::string suffix)
{
#if BOOST_VERSION >= 104100
 using namespace boost::program_options;
 options_description desc;
 bool do_wte;
 bool wte_restart;
 bool use_structure;
 bool use_compact_Cmd1p;
 bool add_Spc42p;
 bool add_Spc29p;
 bool add_Spc110p;
 bool add_Cmd1p;
 bool add_Cnm67p_c;
 bool add_fret;
 bool add_y2h;
 bool add_tilt;
 bool add_GFP;
 bool use_GFP_structure;
 bool keep_GFP_layer;
 std::string cell_type;
 std::string load_Spc42p;
 std::string load_Spc29p;
 std::string load_Spc110p;
 std::string load_Cmd1p;
 std::string load_Cnm67p_c;
 std::map<std::string,std::string> file_list;

 desc.add_options()("do_wte",       value<bool>(&do_wte),           "ciao");
 desc.add_options()("wte_restart",  value<bool>(&wte_restart),      "ciao");
 desc.add_options()("use_structure",value<bool>(&use_structure),    "ciao");
 desc.add_options()("use_compact_Cmd1p",
  value<bool>(&use_compact_Cmd1p), "ciao");
 desc.add_options()("cell_type",    value<std::string >(&cell_type), "ciao");
 desc.add_options()("add_Spc42p",   value<bool>(&add_Spc42p),       "ciao");
 desc.add_options()("add_Spc29p",   value<bool>(&add_Spc29p),       "ciao");
 desc.add_options()("add_Spc110p",  value<bool>(&add_Spc110p),      "ciao");
 desc.add_options()("add_Cmd1p",    value<bool>(&add_Cmd1p),        "ciao");
 desc.add_options()("add_Cnm67p_c", value<bool>(&add_Cnm67p_c),     "ciao");
 desc.add_options()("add_fret",     value<bool>(&add_fret),         "ciao");
 desc.add_options()("add_y2h",      value<bool>(&add_y2h),          "ciao");
 desc.add_options()("add_tilt",     value<bool>(&add_tilt),         "ciao");
 desc.add_options()("add_GFP",      value<bool>(&add_GFP),          "ciao");
 desc.add_options()("use_GFP_structure",
  value<bool>(&use_GFP_structure), "ciao");
 desc.add_options()("keep_GFP_layer",value<bool>(&keep_GFP_layer), "ciao");
 desc.add_options()("load_Spc42p",value<std::string>(&load_Spc42p), "ciao");
 desc.add_options()("load_Spc29p",value<std::string>(&load_Spc29p), "ciao");
 desc.add_options()("load_Spc110p",value<std::string>(&load_Spc110p),"ciao");
 desc.add_options()("load_Cmd1p",value<std::string>(&load_Cmd1p), "ciao");
 desc.add_options()("load_Cnm67p_c",value<std::string>(&load_Cnm67p_c),"ciao");


 OPTION(double, mc_tmin);
 OPTION(double, mc_tmax);
 OPTION(double, mc_dx);
 OPTION(double, mc_dang);
 OPTION(double, kappa);
 OPTION(double, tilt);
 OPTION(double, side);
 OPTION(double, CP_thickness);
 OPTION(double, CP_IL2_gap);
 OPTION(double, IL2_thickness);
 OPTION(double, resolution);
 OPTION(double, wte_w0);
 OPTION(double, wte_sigma);
 OPTION(double, wte_gamma);
 OPTION(double, wte_emin);
 OPTION(double, wte_emax);
 OPTION(int,    mc_nexc);
 OPTION(int,    mc_nsteps);
 OPTION(int,    mc_nhot);
 OPTION(int,    mc_nwrite);

 variables_map vm;
 store(parse_config_file(in.get_stream(), desc, false), vm);
 notify(vm);

 SPBParameters ret;

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
 ret.MC.wte_restart=wte_restart;


// General Parameters
 ret.side=side;
 ret.CP_thickness=CP_thickness;
 ret.CP_IL2_gap=CP_IL2_gap;
 ret.IL2_thickness=IL2_thickness;
 ret.kappa=kappa;
 ret.tilt=radians(tilt);
 ret.cell_type=cell_type;
 ret.use_structure=use_structure;
 ret.use_compact_Cmd1p=use_compact_Cmd1p;
 ret.resolution=resolution;
// restraint
 ret.add_fret=add_fret;
 ret.add_y2h=add_y2h;
 ret.add_tilt=add_tilt;
// protein_list
 ret.protein_list["Spc42p"]=add_Spc42p;
 ret.protein_list["Spc29p"]=add_Spc29p;
 ret.protein_list["Cmd1p"]=add_Cmd1p;
 ret.protein_list["Cnm67p_c"]=add_Cnm67p_c;
 ret.protein_list["Spc110p"]=add_Spc110p;
// GFP stuff
 ret.add_GFP=add_GFP;
 ret.use_GFP_structure=use_GFP_structure;
 ret.keep_GFP_layer=keep_GFP_layer;

// file map
 if(load_Spc42p.length()>0.0){
  ret.file_list["Spc42p"]=load_Spc42p+suffix+".rmf";
  if(add_GFP){
   ret.file_list["Spc42p-N-GFP"]=load_Spc42p+suffix+".rmf";
   ret.file_list["Spc42p-C-GFP"]=load_Spc42p+suffix+".rmf";
  }
 }
 if(load_Spc29p.length()>0.0){
  ret.file_list["Spc29p"]=load_Spc29p+suffix+".rmf";
  if(add_GFP){
   ret.file_list["Spc29p-N-GFP"]=load_Spc29p+suffix+".rmf";
   ret.file_list["Spc29p-C-GFP"]=load_Spc29p+suffix+".rmf";
  }
 }
 if(load_Cmd1p.length()>0.0){
  ret.file_list["Cmd1p"]=load_Cmd1p+suffix+".rmf";
  if(add_GFP){
   ret.file_list["Cmd1p-N-GFP"]=load_Cmd1p+suffix+".rmf";
   ret.file_list["Cmd1p-C-GFP"]=load_Cmd1p+suffix+".rmf";
  }
 }
 if(load_Cnm67p_c.length()>0.0){
  ret.file_list["Cnm67p_c"]=load_Cnm67p_c+suffix+".rmf";
  if(add_GFP){ret.file_list["Cnm67p_c-C-GFP"]=load_Cnm67p_c+suffix+".rmf";}
 }
 if(load_Spc110p.length()>0.0){
  ret.file_list["Spc110p"]=load_Spc110p+suffix+".rmf";
  if(add_GFP){ret.file_list["Spc110p-C-GFP"]=load_Spc110p+suffix+".rmf";}
 }

 algebra::Vector3D CP_center;
 algebra::Vector3D IL2_center;
 double dz=CP_IL2_gap+IL2_thickness/2.0+CP_thickness/2.0;
// cell dependent parameters
 if(cell_type=="rhombus"){
  ret.num_cells=21;
  ret.num_copies=2;
  CP_center=algebra::Vector3D(side/4.0,side/4.0*sqrt(3.0),0.0);
  IL2_center=algebra::Vector3D(side/4.0,side/4.0*sqrt(3.0),dz);
 }else if(cell_type=="hexagon"){
  ret.num_cells=7;
  ret.num_copies=6;
  CP_center=algebra::Vector3D(0.0,0.0,0.0);
  IL2_center=algebra::Vector3D(0.0,0.0,dz);
 }else if(cell_type=="square"){
  ret.num_cells=9;
  ret.num_copies=6;
  side=sqrt(1.5*pow(side,2)*sqrt(3.0));
  ret.side=side;
  CP_center=algebra::Vector3D(0.0,0.0,0.0);
  IL2_center=algebra::Vector3D(0.0,0.0,dz);
 }else{
  IMP_FAILURE("Unknown cell type!");
 }

// storing cell centers and transformations
 algebra::Vector3Ds translations;
 int num_rotations;
 double angle;

 if(cell_type!="square"){
  translations.push_back(algebra::Vector3D(0.0,0.0,0.0));
  translations.push_back(algebra::Vector3D(0.0,side*sqrt(3.0),0.0));
  translations.push_back(algebra::Vector3D(1.5*side,side*sqrt(3.0)/2.0,0.0));
  translations.push_back(algebra::Vector3D(1.5*side,-side*sqrt(3.0)/2.0,0.0));
  translations.push_back(algebra::Vector3D(0.0,-side*sqrt(3.0),0.0));
  translations.push_back(algebra::Vector3D(-1.5*side,-side*sqrt(3.0)/2.0,0.0));
  translations.push_back(algebra::Vector3D(-1.5*side,side*sqrt(3.0)/2.0,0.0));
  num_rotations=ret.num_cells/7;
  angle=2.0*IMP::PI/(double)num_rotations;
 }else{
  translations.push_back(algebra::Vector3D(0.0,0.0,0.0));
  translations.push_back(algebra::Vector3D(0.0,side,0.0));
  translations.push_back(algebra::Vector3D(0.0,-side,0.0));
  translations.push_back(algebra::Vector3D(side,0.0,0.0));
  translations.push_back(algebra::Vector3D(side,side,0.0));
  translations.push_back(algebra::Vector3D(side,-side,0.0));
  translations.push_back(algebra::Vector3D(-side,0.0,0.0));
  translations.push_back(algebra::Vector3D(-side,side,0.0));
  translations.push_back(algebra::Vector3D(-side,-side,0.0));
  num_rotations=1;
  angle=0.0;
 }
 for(unsigned int i=0;i<translations.size();++i){
  for(int j=0;j<num_rotations;++j){
   algebra::Rotation3D rot=algebra::get_rotation_about_axis
   (algebra::Vector3D(0.0,0.0,1.0), (double) j * angle);
   algebra::Transformation3D tr=algebra::Transformation3D(rot,translations[i]);
   ret.trs.push_back(tr);
   ret.CP_centers.push_back(tr.get_transformed(CP_center));
   ret.IL2_centers.push_back(tr.get_transformed(IL2_center));
  }
 }


 return ret;
#else
 IMP_FAILURE("Need newer boost");
#endif
}

IMPMEMBRANE_END_NAMESPACE
