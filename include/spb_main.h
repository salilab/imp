/**
 *  \file  spb_main.h
 *  \brief SPB MonteCarlo
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_MAIN_H
#define IMPMEMBRANE_SPB_MAIN_H
#include "membrane_config.h"
#include <IMP.h>
#include <string>
#include <map>

IMPMEMBRANE_BEGIN_NAMESPACE

// convert degrees to radians
double radians (double d);
inline double radians (double d) {return d * IMP::algebra::PI / 180.0;}

struct MCParameters {
 double         tmin;
 double         tmax;
 double         dx;
 double         dang;
 double         dSigma0;
 double         dKda;
 double         dIda;
 double         dA;
 double         dSide;
 double         dR0;
 double         dpBl;
 double         wte_w0;
 double         wte_sigma;
 double         wte_gamma;
 double         wte_emin;
 double         wte_emax;
 int            nexc;
 int            nsteps;
 int            nwrite;
 int            nhot;
 bool           do_wte;
 bool           wte_restart;
};

struct EM2DParameters {
 double resolution;
 double pixel_size;
 std::string filename;
 double SigmaMin;
 double SigmaMax;
};

struct FretParameters {
 double R0Min;
 double R0Max;
 double KdaMin;
 double KdaMax;
 double Ida;
 double IdaErr;
 double Sigma0Min;
 double Sigma0Max;
 double pBlMin;
 double pBlMax;
 double KdaMin_new;
 double KdaMax_new;
 double Ida_new;
 double IdaErr_new;
 std::string filename;
 std::string filename_new;
};

struct ClusterParameters {
 std::string trajfile;
 std::string isdtrajfile;
 std::string biasfile;
 std::string weightfile;
 std::string label;
 int niter;
 double cutoff;
 bool weight;
 bool identical;
 bool symmetry;
};

struct MapParameters {
 std::string frames_list;
 std::string label;
 std::string ref_file;
 std::string ref_isdfile;
 double xmin;
 double xmax;
 double ymin;
 double ymax;
 double zmin;
 double zmax;
 double resolution;
 int nbinx;
 int nbiny;
 int nbinz;
};

struct SPBParameters {
 MCParameters MC;
 FretParameters Fret;
 EM2DParameters EM2D;
 ClusterParameters Cluster;
 MapParameters Map;
 double       sideMin;
 double       sideMax;
 double       CP_thicknessMin;
 double       CP_thicknessMax;
 double       CP_IL2_gapMin;
 double       CP_IL2_gapMax;
 double       IL2_thickness;
 double       Spc29_diameter;
 double       Spc29_rgyr;
 double       kappa;
 double       kappa_vol;
 double       tilt;
 double       tilt_Spc42;
 double       resolution;
 std::string  cell_type;
 int          num_cells;
 int          num_copies;
 algebra::Vector3Ds CP_centers;
 algebra::Vector3Ds IL2_centers;
 algebra::Transformation3Ds trs;
 bool         use_connectivity;
 std::map<std::string,bool> protein_list;
 bool         add_fret;
 bool         add_new_fret;
 bool         add_y2h;
 bool         add_tilt;
 bool         add_GFP;
 bool         use_GFP_structure;
 bool         keep_GFP_layer;
 bool         GFP_exc_volume;
 bool         fix_GFP;
 bool         restraint_GFP;
 std::map<std::string,std::string> file_list;
 bool         isd_restart;
 std::string  isd_restart_file;
 bool         add_IL2_layer;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_MAIN_H */
