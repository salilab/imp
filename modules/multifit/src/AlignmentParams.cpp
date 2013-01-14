/**
 *  \file AlignmentParams.cpp
 *  \brief Parameters for alignments.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <IMP/multifit/AlignmentParams.h>
#include <IMP/base/check_macros.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

AlignmentParams::AlignmentParams(const char* param_fn) {
  boost::property_tree::ptree pt;

  boost::property_tree::read_ini(param_fn, pt);
  fitting_params_.add(pt);
  complementarity_params_.add(pt);
  domino_params_.add(pt);
  xlink_params_.add(pt);
  conn_params_.add(pt);
  rog_params_.add(pt);
  filters_params_.add(pt);
  ev_params_.add(pt);
  fragments_params_.add(pt);
}

IMPMULTIFIT_END_NAMESPACE
