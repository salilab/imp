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

struct FretParameters {
 double R0;
 double Gamma;
 double Sd;
 double Sa;
 double Ida;
 std::string filename;
};

struct SPBParameters {
 MCParameters MC;
 FretParameters Fret;
 double       side;
 double       CP_thickness;
 double       CP_IL2_gap;
 double       IL2_thickness;
 double       kappa;
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
 int          num_cells;
 int          num_copies;
 algebra::Vector3Ds CP_centers;
 algebra::Vector3Ds IL2_centers;
 algebra::Transformation3Ds trs;
 bool         use_structure;
 bool         use_compact_Cmd1p;
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
 bool         cluster_time;
 bool         cluster_weight;
 std::map<std::string,std::string> file_list;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_MAIN_H */
