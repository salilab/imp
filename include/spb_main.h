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
inline double radians (double d) {return d * IMP::algebra::PI/ 180.0;}

struct MCParameters {
 double         tmin;
 double         tmax;
 double         dx;
 double         dang;
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

struct FretParameters {
 double R0;
 double Gamma;
 double Sd;
 double Sa;
 double Ida;
 double Pbleach0;
 double Pbleach1;
 std::string filename;
 bool   sigmafilter;
 double sigmamult;
 Floats sigmas;
};

struct SPBParameters {
 MCParameters MC;
 FretParameters Fret;
 double       side;
 double       CP_thickness;
 double       CP_IL2_gap;
 double       IL2_thickness;
 double       kappa;
 double       kappa_vol;
 double       tilt;
 double       resolution;
 double       cutoff;
 double       cluster_cut;
 std::string  cell_type;
 std::string  trajfile;
 std::string  label;
 int          cg_steps;
 int          nrep;
 int          niter;
 int          chunk;
 int          replica_index;
 int          num_cells;
 int          num_copies;
 int          frame_id;
 algebra::Vector3Ds CP_centers;
 algebra::Vector3Ds IL2_centers;
 algebra::Transformation3Ds trs;
 bool         use_connectivity;
 bool         use_new_fret_model;
 std::map<std::string,bool> protein_list;
 bool         add_fret;
 bool         add_y2h;
 bool         add_tilt;
 bool         add_GFP;
 bool         use_GFP_structure;
 bool         keep_GFP_layer;
 bool         GFP_exc_volume;
 bool         fix_GFP;
 bool         restraint_GFP;
 bool         cluster_weight;
 std::map<std::string,std::string> file_list;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_MAIN_H */
