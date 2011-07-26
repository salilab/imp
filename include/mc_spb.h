/**
 *  \file mc_spb.h
 *  \brief SPB MonteCarlo
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_MC_SPB_H
#define IMPMEMBRANE_MC_SPB_H
#include "membrane_config.h"
#include <IMP.h>
#include <string>

IMPMEMBRANE_BEGIN_NAMESPACE

struct SPBParameters {
 MCParameters   MC;
 double         side;
 double         kappa;
 double         resolution;
 std::string    cell_type;
 int            num_cells;
 int            num_copies;
 bool           use_structure;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MC_SPB_H */
