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

using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

#define OPTION(type, name)                                   \
 type name=-1;                                              \
 desc.add_options()(#name, value< type >(&name), #name);

#define CHECK(type, name)                                    \
 IMP_USAGE_CHECK(name >=0, #name << " is " << name);


SPBParameters get_SPBParameters(TextInput in) {
#if BOOST_VERSION >= 104100
 using namespace boost::program_options;
 options_description desc;
 bool do_wte,use_structure;
 std::string cell_type;

 desc.add_options()("do_wte",       value< bool >(&do_wte),           "ciao");
 desc.add_options()("use_structure",value< bool >(&use_structure),    "ciao");
 desc.add_options()("cell_type",    value< std::string >(&cell_type), "ciao");

 OPTION(double, mc_tmin);
 OPTION(double, mc_tmax);
 OPTION(double, mc_dx);
 OPTION(double, mc_dang);
 OPTION(double, kappa);
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

 CHECK(double, mc_tmin);
 CHECK(double, mc_tmax);
 CHECK(double, mc_dx);
 CHECK(double, mc_dang);
 CHECK(double, kappa);
 CHECK(double, CP_thickness);
 CHECK(double, CP_IL2_gap);
 CHECK(double, IL2_thickness);
 CHECK(double, side);
 CHECK(double, resolution);
 CHECK(double, wte_w0);
 CHECK(double, wte_sigma);
 CHECK(double, wte_gamma);
 CHECK(double, wte_emin);
 CHECK(double, wte_emax);
 CHECK(int,    mc_nexc);
 CHECK(int,    mc_nsteps);
 CHECK(int,    mc_nhot);
 CHECK(int,    mc_nwrite);

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


// General Parameters
 ret.side=side;
 ret.CP_thickness=CP_thickness;
 ret.CP_IL2_gap=CP_IL2_gap;
 ret.IL2_thickness=IL2_thickness;
 ret.kappa=kappa;
 ret.cell_type=cell_type;
 ret.use_structure=use_structure;
 ret.resolution=resolution;

// cell dependent parameters
 if(cell_type=="triangle"){
  ret.num_cells=42;
  ret.num_copies=1;
 } else if(cell_type=="rhombus"){
  ret.num_cells=21;
  ret.num_copies=2;
 }else if(cell_type=="hexagon"){
  ret.num_cells=7;
  ret.num_copies=6;
 }else if(cell_type=="square"){
  ret.num_cells=9;
  ret.num_copies=6;
  ret.side=sqrt(1.5*pow(side,2)*sqrt(3.0));
 }else{
  IMP_FAILURE("Unknown cell type!");
 }

 return ret;
#else
 IMP_FAILURE("Need newer boost");
#endif
}

IMPMEMBRANE_END_NAMESPACE
