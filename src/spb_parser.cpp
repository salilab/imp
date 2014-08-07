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

SPBParameters get_SPBParameters(base::TextInput in, std::string suffix)
{
#if BOOST_VERSION >= 104100
 using namespace boost::program_options;
 options_description desc;
 bool do_wte;
 bool wte_restart;
 bool add_Spc42p;
 bool add_Spc29p;
 bool add_Spc110p;
 bool add_Cmd1p;
 bool add_Cnm67p;
 bool add_fret;
 bool add_new_fret;
 bool add_y2h;
 bool add_tilt;
 bool add_GFP;
 bool use_GFP_structure;
 bool keep_GFP_layer;
 bool GFP_exc_volume;
 bool fix_GFP;
 bool restraint_GFP;
 bool use_connectivity;
 bool cluster_weight;
 bool cluster_identical;
 bool cluster_symmetry;
 bool isd_restart;
 bool add_IL2_layer;
 std::string isd_restart_file;
 std::string cell_type;
 std::string load_Spc42p;
 std::string load_Spc29p;
 std::string load_Spc110p;
 std::string load_Cmd1p;
 std::string load_Cnm67p;
 std::string cluster_trajfile;
 std::string cluster_isdtrajfile;
 std::string cluster_biasfile;
 std::string cluster_label;
 std::string cluster_weightfile;
 std::string fret_File;
 std::string fret_new_File;
 std::string EM2D_File;
 std::string map_frames_list;
 std::string map_label;
 std::string map_ref_file;
 std::string map_ref_isdfile;
 std::map<std::string,std::string> file_list;

 desc.add_options()("do_wte",       value<bool>(&do_wte),           "ciao");
 desc.add_options()("wte_restart",  value<bool>(&wte_restart),      "ciao");
 desc.add_options()("cell_type",    value<std::string >(&cell_type), "ciao");
 desc.add_options()("add_Spc42p",   value<bool>(&add_Spc42p),       "ciao");
 desc.add_options()("add_Spc29p",   value<bool>(&add_Spc29p),       "ciao");
 desc.add_options()("add_Spc110p",  value<bool>(&add_Spc110p),      "ciao");
 desc.add_options()("add_Cmd1p",    value<bool>(&add_Cmd1p),        "ciao");
 desc.add_options()("add_Cnm67p",   value<bool>(&add_Cnm67p),       "ciao");
 desc.add_options()("add_fret",     value<bool>(&add_fret),         "ciao");
 desc.add_options()("add_new_fret", value<bool>(&add_new_fret),     "ciao");
 desc.add_options()("add_y2h",      value<bool>(&add_y2h),          "ciao");
 desc.add_options()("add_tilt",     value<bool>(&add_tilt),         "ciao");
 desc.add_options()("add_GFP",      value<bool>(&add_GFP),          "ciao");
 desc.add_options()("use_GFP_structure",
  value<bool>(&use_GFP_structure), "ciao");
 desc.add_options()("GFP_exc_volume",value<bool>(&GFP_exc_volume),      "ciao");
 desc.add_options()("keep_GFP_layer",value<bool>(&keep_GFP_layer),      "ciao");
 desc.add_options()("fix_GFP",       value<bool>(&fix_GFP),             "ciao");
 desc.add_options()("restraint_GFP", value<bool>(&restraint_GFP),       "ciao");
 desc.add_options()("use_connectivity",value<bool>(&use_connectivity),  "ciao");
 desc.add_options()("cluster_weight",value<bool>(&cluster_weight),      "ciao");
 desc.add_options()("cluster_identical",value<bool>(&cluster_identical),"ciao");
 desc.add_options()("cluster_symmetry", value<bool>(&cluster_symmetry), "ciao");
 desc.add_options()("load_Spc42p",   value<std::string>(&load_Spc42p),  "ciao");
 desc.add_options()("load_Spc29p",   value<std::string>(&load_Spc29p),  "ciao");
 desc.add_options()("load_Spc110p",  value<std::string>(&load_Spc110p), "ciao");
 desc.add_options()("load_Cmd1p",    value<std::string>(&load_Cmd1p),   "ciao");
 desc.add_options()("load_Cnm67p",   value<std::string>(&load_Cnm67p),  "ciao");
 desc.add_options()("cluster_trajfile",
                               value<std::string>(&cluster_trajfile),   "ciao");
 desc.add_options()("cluster_isdtrajfile",
                             value<std::string>(&cluster_isdtrajfile),  "ciao");
 desc.add_options()("cluster_biasfile",
                             value<std::string>(&cluster_biasfile),     "ciao");
 desc.add_options()("cluster_label",
                             value<std::string>(&cluster_label),        "ciao");
 desc.add_options()("cluster_weightfile",
                             value<std::string>(&cluster_weightfile),   "ciao");
 desc.add_options()("fret_File",     value<std::string>(&fret_File),    "ciao");
 desc.add_options()("fret_new_File", value<std::string>(&fret_new_File),"ciao");
 desc.add_options()("EM2D_File",     value<std::string>(&EM2D_File),    "ciao");
 desc.add_options()("isd_restart",   value<bool>(&isd_restart),         "ciao");
 desc.add_options()("isd_restart_file",
                                value<std::string >(&isd_restart_file), "ciao");
 desc.add_options()("add_IL2_layer", value<bool>(&add_IL2_layer),       "ciao");

 desc.add_options()("map_frames_list",
                                   value<std::string>(&map_frames_list),"ciao");
 desc.add_options()("map_label",     value<std::string>(&map_label),    "ciao");
 desc.add_options()("map_ref_file",  value<std::string>(&map_ref_file), "ciao");
 desc.add_options()("map_ref_isdfile",
                                  value<std::string>(&map_ref_isdfile),"ciao");

 OPTION(double, mc_tmin);
 OPTION(double, mc_tmax);
 OPTION(double, mc_dx);
 OPTION(double, mc_dang);
 OPTION(double, mc_dKda);
 OPTION(double, mc_dIda);
 OPTION(double, mc_dSigma0);
 OPTION(double, mc_dA);
 OPTION(double, mc_dSide);
 OPTION(double, mc_dR0);
 OPTION(double, mc_dpBl);
 OPTION(double, kappa);
 OPTION(double, kappa_vol);
 OPTION(double, tilt);
 OPTION(double, tilt_Spc42);
 OPTION(double, sideMin);
 OPTION(double, sideMax);
 OPTION(double, CP_thicknessMin);
 OPTION(double, CP_thicknessMax);
 OPTION(double, CP_IL2_gapMin);
 OPTION(double, CP_IL2_gapMax);
 OPTION(double, IL2_thickness);
 OPTION(double, Spc29_diameter);
 OPTION(double, Spc29_rgyr);
 OPTION(double, resolution);
 OPTION(double, wte_w0);
 OPTION(double, wte_sigma);
 OPTION(double, wte_gamma);
 OPTION(double, wte_emin);
 OPTION(double, wte_emax);
 OPTION(double, cluster_cutoff);
 OPTION(double, fret_R0Min);
 OPTION(double, fret_R0Max);
 OPTION(double, fret_KdaMin);
 OPTION(double, fret_KdaMax);
 OPTION(double, fret_Ida);
 OPTION(double, fret_IdaErr);
 OPTION(double, fret_Sigma0Min);
 OPTION(double, fret_Sigma0Max);
 OPTION(double, fret_pBlMin);
 OPTION(double, fret_pBlMax);
 OPTION(double, fret_new_KdaMin);
 OPTION(double, fret_new_KdaMax);
 OPTION(double, fret_new_Ida);
 OPTION(double, fret_new_IdaErr);
 OPTION(double, EM2D_resolution);
 OPTION(double, EM2D_pixel_size);
 OPTION(double, EM2D_SigmaMin);
 OPTION(double, EM2D_SigmaMax);
 OPTION(double, map_resolution);
 OPTION(int,    cluster_niter);
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
 ret.MC.dKda=mc_dKda;
 ret.MC.dIda=mc_dIda;
 ret.MC.dSigma0=mc_dSigma0;
 ret.MC.dA=mc_dA;
 ret.MC.dSide=mc_dSide;
 ret.MC.dR0=mc_dR0;
 ret.MC.dpBl=mc_dpBl;
// Wte Parameters
 ret.MC.do_wte=do_wte;
 ret.MC.wte_w0=wte_w0;
 ret.MC.wte_sigma=wte_sigma;
 ret.MC.wte_gamma=wte_gamma;
 ret.MC.wte_emin=wte_emin;
 ret.MC.wte_emax=wte_emax;
 ret.MC.wte_restart=wte_restart;

// Fret Parameters
 ret.Fret.R0Min=fret_R0Min;
 ret.Fret.R0Max=fret_R0Max;
 ret.Fret.KdaMin=fret_KdaMin;
 ret.Fret.KdaMax=fret_KdaMax;
 ret.Fret.Ida=fret_Ida;
 ret.Fret.IdaErr=fret_IdaErr;
 ret.Fret.Sigma0Min=fret_Sigma0Min;
 ret.Fret.Sigma0Max=fret_Sigma0Max;
 ret.Fret.pBlMin=fret_pBlMin;
 ret.Fret.pBlMax=fret_pBlMax;
 ret.Fret.filename=fret_File;
// New fret
 ret.Fret.filename_new=fret_new_File;
 ret.Fret.KdaMin_new=fret_new_KdaMin;
 ret.Fret.KdaMax_new=fret_new_KdaMax;
 ret.Fret.Ida_new=fret_new_Ida;
 ret.Fret.IdaErr_new=fret_new_IdaErr;

// EM2D
 ret.EM2D.resolution=EM2D_resolution;
 ret.EM2D.pixel_size=EM2D_pixel_size;
 ret.EM2D.filename=EM2D_File;
 ret.EM2D.SigmaMin=EM2D_SigmaMin;
 ret.EM2D.SigmaMax=EM2D_SigmaMax;

// density maps plot
 ret.Map.frames_list = map_frames_list;
 ret.Map.label       = map_label;
 ret.Map.ref_file    = map_ref_file;
 ret.Map.ref_isdfile = map_ref_isdfile;
 ret.Map.resolution  = map_resolution;

// General Parameters
 ret.sideMin=sideMin;
 ret.sideMax=sideMax;
 ret.CP_thicknessMin=CP_thicknessMin;
 ret.CP_thicknessMax=CP_thicknessMax;
 ret.CP_IL2_gapMin=CP_IL2_gapMin;
 ret.CP_IL2_gapMax=CP_IL2_gapMax;
 ret.IL2_thickness=IL2_thickness;
 ret.Spc29_diameter=Spc29_diameter;
 ret.Spc29_rgyr=Spc29_rgyr;
 ret.kappa=kappa;
 ret.kappa_vol=kappa_vol;
 ret.tilt=radians(tilt);
 ret.tilt_Spc42=radians(tilt_Spc42);
 ret.cell_type=cell_type;
 ret.resolution=resolution;
 ret.use_connectivity=use_connectivity;
 ret.add_IL2_layer=add_IL2_layer;

// ISD stuff
 ret.isd_restart=isd_restart;
 ret.isd_restart_file=isd_restart_file+suffix+".rmf";

// Clustering parameters
 ret.Cluster.trajfile=cluster_trajfile;
 ret.Cluster.isdtrajfile=cluster_isdtrajfile;
 ret.Cluster.biasfile=cluster_biasfile;
 ret.Cluster.label=cluster_label;
 ret.Cluster.weightfile=cluster_weightfile;
 ret.Cluster.niter=cluster_niter;
 ret.Cluster.cutoff=cluster_cutoff;
 ret.Cluster.weight=cluster_weight;
 ret.Cluster.identical=cluster_identical;
 ret.Cluster.symmetry=cluster_symmetry;

// Restraints
 ret.add_fret=add_fret;
 ret.add_new_fret=add_new_fret;
 ret.add_y2h=add_y2h;
 ret.add_tilt=add_tilt;

// protein_list
 ret.protein_list["Spc42p"]=add_Spc42p;
 ret.protein_list["Spc29p"]=add_Spc29p;
 ret.protein_list["Cmd1p"]=add_Cmd1p;
 ret.protein_list["Cnm67p"]=add_Cnm67p;
 ret.protein_list["Spc110p"]=add_Spc110p;

// GFP stuff
 ret.add_GFP=add_GFP;
 ret.use_GFP_structure=use_GFP_structure;
 ret.keep_GFP_layer=keep_GFP_layer;
 ret.GFP_exc_volume=GFP_exc_volume;
 ret.fix_GFP=fix_GFP;
 ret.restraint_GFP=restraint_GFP;

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
 if(load_Cnm67p.length()>0.0){
  ret.file_list["Cnm67p"]=load_Cnm67p+suffix+".rmf";
  if(add_GFP){ret.file_list["Cnm67p-C-GFP"]=load_Cnm67p+suffix+".rmf";}
 }
 if(load_Spc110p.length()>0.0){
  ret.file_list["Spc110p"]=load_Spc110p+suffix+".rmf";
  if(add_GFP){ret.file_list["Spc110p-C-GFP"]=load_Spc110p+suffix+".rmf";}
 }

 algebra::Vector3D CP_center;
 algebra::Vector3D IL2_center;
 double dz=CP_IL2_gapMax;
 double side=sideMin;

// cell dependent parameters
 if(cell_type=="rhombus"){
  ret.num_cells=21;
  ret.num_copies=2;
  CP_center =algebra::Vector3D(side/4.0,side/4.0*sqrt(3.0),0.0);
  IL2_center=algebra::Vector3D(side/4.0,side/4.0*sqrt(3.0),dz);
 }else if(cell_type=="hexagon"){
  ret.num_cells=7;
  ret.num_copies=6;
  CP_center =algebra::Vector3D(0.0,0.0,0.0);
  IL2_center=algebra::Vector3D(0.0,0.0,dz);
 }else if(cell_type=="square"){
  ret.num_cells=9;
  ret.num_copies=6;
  ret.sideMin=sqrt(1.5*pow(ret.sideMin,2)*sqrt(3.0));
  ret.sideMax=sqrt(1.5*pow(ret.sideMax,2)*sqrt(3.0));
  CP_center =algebra::Vector3D(0.0,0.0,0.0);
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
  angle=2.0*IMP::algebra::PI/(double)num_rotations;
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
