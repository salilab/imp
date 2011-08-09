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

struct SPBParameters {
 MCParameters   MC;
 double         side;
 double         CP_thickness;
 double         CP_IL2_gap;
 double         IL2_thickness;
 double         kappa;
 double         tilt;
 double         resolution;
 std::string    cell_type;
 int            num_cells;
 int            num_copies;
 algebra::Vector3Ds CP_centers;
 algebra::Vector3Ds IL2_centers;
 algebra::Transformation3Ds trs;
 bool           use_structure;
 std::map<std::string,bool> protein_list;
 bool           add_fret;
 bool           add_y2h;
 std::map<std::string,std::string> file_list;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_MAIN_H */
