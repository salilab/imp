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

IMPMEMBRANE_BEGIN_NAMESPACE

// convert degrees to radians
double radians (double d);
inline double radians (double d) {return d * IMP::PI/ 180.0;}

// grid structure
struct GridParameters {
  double tilt;
  double swing;
  double rot;
  double x;
  double tiltmax;
  double swingmax;
  double rotmax;
  double xmax;
  double zmax;
};

struct HelixData {
 int num;
 std::vector<char>                 seq;
 std::vector<double>               topo;
 std::vector<std::pair<int,int> >  resid;
 std::vector<std::string>          name;
 std::vector<std::string>          struct_file;
 std::vector<std::pair<int,int> >  inter;
 std::vector<std::pair<int,int> >  loop;
};

struct MCParameters {
 double         tmin;
 double         tmax;
 double         dx;
 double         dang;
 int            nexc;
 int            nsteps;
 int            nhot;
 bool           do_wte;
};

struct Parameters {
 GridParameters grid;
 HelixData      TM;
 MCParameters   MC;
 double         diameter;
 std::string    score_name;
 std::string    ass_file;
 std::string    traj_file;
 bool           add_dope;
 bool           add_pack;
 bool           use_volume;
};

//general parameters for restraints
const double kappa_=100.0;
const double max_score_=0.01;

// packing restraint cluster stuff
const int packing_nsig_=3;
const int packing_ncl_=13;

// TODO: move this stuff into HelixData
// depth range
const IMP::FloatRange z_range_ = IMP::FloatRange(-5.0,5.0);
// tilt range
const IMP::FloatRange tilt_range_ = IMP::FloatRange(0.0,radians(50.0));

// other restraints
const double d0_inter_=8.0;
const double cm_dist_=25.0;

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_DOMINO_ENUMERATE_H */
