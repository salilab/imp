/**
 *  \file  membrane_main.h
 *  \brief Membrane sampling stuff
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_MEMBRANE_MAIN_H
#define IMPMEMBRANE_MEMBRANE_MAIN_H
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

struct RstParameters {
 double         diameter;
 double         tilt;
 double         zeta;
 double         d0_inter;
 double         cm_dist;
 std::string    score_name;
 std::string    saxs_profile;
 bool           add_dope;
 bool           use_volume;
 double         k_depth;
 double         k_tilt;
 double         k_volume;
 double         k_pack;
 double         k_diameter;
 double         k_endtoend;
 double         k_cmdist;
 double         k_inter;
 double         k_saxs;
};

struct Parameters {
 GridParameters grid;
 HelixData      TM;
 MCParameters   MC;
 RstParameters  RST;
 std::string    ass_file;
 std::string    traj_file;
 bool           do_random;
 std::string    reload;
};

// packing restraint cluster stuff
const int packing_nsig_=3;
const int packing_ncl_=13;

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MEMBRANE_MAIN_H */
