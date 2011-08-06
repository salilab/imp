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

IMPMEMBRANE_BEGIN_NAMESPACE

struct SPBParameters {
 MCParameters   MC;
 double         side;
 double         CP_thickness;
 double         CP_IL2_gap;
 double         IL2_thickness;
 double         kappa;
 double         resolution;
 std::string    cell_type;
 int            num_cells;
 int            num_copies;
 algebra::Vector3Ds CP_centers;
 algebra::Vector3Ds IL2_centers;
 algebra::Transformation3Ds trs;
 bool           use_structure;
 bool           add_Spc42p;
 bool           add_Spc29p;
 bool           add_Spc110p;
 bool           add_Cmd1p;
 bool           add_Cnm67p_c;
 bool           add_fret;
 bool           add_y2h;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SPB_MAIN_H */
