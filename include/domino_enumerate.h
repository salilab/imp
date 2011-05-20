/**
 *  \file domino_enumerate.h
 *  \brief Membrane domino stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_DOMINO_ENUMERATE_H
#define IMPMEMBRANE_DOMINO_ENUMERATE_H
#include "membrane_config.h"
#include <IMP.h>
#include <string>
#include <vector>

IMPMEMBRANE_BEGIN_NAMESPACE

// convert degrees to radians
double radians (double d);
inline double radians (double d) {return d * IMP::PI/ 180.0;}

// various parameters are specified here
//general parameters for restraints
const double kappa_=1000.0;
const double max_score_=0.01;

// packing restraint cluster stuff
const int packing_nsig_=3;
const int packing_ncl_=13;

// depth range
const IMP::FloatRange z_range_ = IMP::FloatRange(-5.0,5.0);

// tilt range
const IMP::FloatRange tilt_range_ = IMP::FloatRange(0.0,radians(40.0));

// scoring function
std::string score_name_="dope_score.lib";

// other restraints
const double d0_inter_=8.0;
// max diameter
// nTMH::  2   3   4   5   6   7   8   9   10   11   12   13   14
// d   :: 16  35  26  51  48  51  50  34   39   52   48   43   42
const double diameter_=35.0;

// grid
// bin size
const double grid_Dtilt=radians(15.0);
const double grid_Dswing=radians(45.0);
const double grid_Drot=radians(45.0);
const double grid_Dx=1.0;
// nbin
const int grid_itilt=int(radians(45.0)/grid_Dtilt);
const int grid_iswing=int(radians(360.0)/grid_Dswing);
const int grid_irot=int(radians(45.0)/grid_Drot);
const int grid_ix=int(diameter_/grid_Dx);
const int grid_iy=int(diameter_/grid_Dx);
const int grid_iz=int(5.0/grid_Dx);

// assignement file
const std::string ass_file="assign_0.rmf";

//protein data
//sequence
const char TM_seq[] =
{'M','V','G','L','T','T','L','F','W','L','G','A','I','G','M','L','V','G'
,'T','L','A','F','A','W','A','G','R','D','A','G','S','G','E','R','R','Y'
,'Y','V','T','L','V','G','I','S','G','I','A','A','V','A','Y','V','V','M'
,'A','L','G','V','G','W','V','P','V','A','E','R','T','V','F','A','P','R'
,'Y','I','D','W','I','L','T','T','P','L','I','V','Y','F','L','G','L','L'
,'A','G','L','D','S','R','E','F','G','I','V','I','T','L','N','T','V','V'
,'M','L','A','G','F','A','G','A','M','V','P','G','I','E','R','Y','A','L'
,'F','G','M','G','A','V','A','F','L','G','L','V','Y','Y','L','V','G','P'
,'M','T','E','S','A','S','Q','R','S','S','G','I','K','S','L','Y','V','R'
,'L','R','N','L','T','V','I','L','W','A','I','Y','P','F','I','W','L','L'
,'G','P','P','G','V','A','L','L','T','P','T','V','D','V','A','L','I','V'
,'Y','L','D','L','V','T','K','V','G','F','G','F','I','A','L','D','A','A'
,'A','T','L'};

// number of TM
const int TM_num=2;
// TM regions
const int TM_res[TM_num][2] = {{121,141},{163,180}};
// define the topology
const double TM_topo[TM_num]={-1.0,1.0};
// name of the TMH
const std::string TM_names[TM_num]={"TM4","TM5"};
// initial rotation
std::vector<double> TM_rot0;
// interacting pairs
const int TM_ninter=1;
const int TM_inter[TM_ninter][2]={{0,1}};
// adjacent pairs
const int TM_nloop=1;
const int TM_loop[TM_nloop][2]={{0,1}};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_DOMINO_ENUMERATE_H */
