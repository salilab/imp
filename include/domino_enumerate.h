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

struct RstParameters {
 double         diameter;
 double         tilt;
 double         zeta;
 double         kappa;
 double         d0_inter;
 double         cm_dist;
 std::string    score_name;
 bool           add_dope;
 bool           add_pack;
 bool           use_volume;
};

struct Parameters {
 GridParameters grid;
 HelixData      TM;
 MCParameters   MC;
 RstParameters  RST;
 std::string    ass_file;
 std::string    traj_file;
};

//general parameters for restraints
const double max_score_=0.01;

// packing restraint cluster stuff
const int packing_nsig_=3;
const int packing_ncl_=13;

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_DOMINO_ENUMERATE_H */
