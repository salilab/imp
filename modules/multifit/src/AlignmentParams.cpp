/**
 *  \file AlignmentParams.cpp
 *  \brief Parameters for alignments.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <IMP/multifit/AlignmentParams.h>
#include <IMP/base/check_macros.h>
#include <libTAU/Parameters.h>

IMPMULTIFIT_BEGIN_NAMESPACE

AlignmentParams::AlignmentParams(const char* param_fn) {
  std::ifstream params(param_fn);
  TAU::Parameters::readParameters(params);
  params.close();
}

bool AlignmentParams::process_parameters() {
  get_parameters();
  if(!parse_parameters_strings())
    return false;
  return true;
}


void AlignmentParams::get_parameters() {
  if(TAU::Parameters::paramType("fitting") !=
     TAU::Parameters::Undef) {
    fitting_str_ = TAU::Parameters::getString("fitting");
  } else {
    IMP_THROW("Can't find fitting parameters", ValueException);
  }
  if(TAU::Parameters::paramType("complementarity") !=
     TAU::Parameters::Undef) {
    complementarity_str_ = TAU::Parameters::getString("complementarity");
  } else {
    IMP_THROW("Can't find complementarity parameters", ValueException);
  }
  if(TAU::Parameters::paramType("domino") !=
     TAU::Parameters::Undef) {
    domino_str_ = TAU::Parameters::getString("domino");
  } else {
    IMP_THROW("Can't find domino parameters", ValueException);
  }
  if(TAU::Parameters::paramType("xlink") !=
     TAU::Parameters::Undef) {
    xlink_str_ = TAU::Parameters::getString("xlink");
  } else {
    IMP_THROW("Can't find xlink parameters", ValueException);
  }
  if(TAU::Parameters::paramType("conn") !=
     TAU::Parameters::Undef) {
    conn_str_ = TAU::Parameters::getString("conn");
  } else {
    IMP_THROW("Can't find conn parameters", ValueException);
  }
  if(TAU::Parameters::paramType("rog") !=
     TAU::Parameters::Undef) {
    rog_str_ = TAU::Parameters::getString("rog");
  } else {
    IMP_THROW("Can't find rog parameters", ValueException);
  }
  if(TAU::Parameters::paramType("fragments") !=
     TAU::Parameters::Undef) {
    fragments_str_ = TAU::Parameters::getString("fragments");
  } else {
    IMP_THROW("Can't find fragments parameters", ValueException);
  }
  if(TAU::Parameters::paramType("filters") !=
     TAU::Parameters::Undef) {
    filters_str_ = TAU::Parameters::getString("filters");
  } else {
    IMP_THROW("Can't find filters parameters", ValueException);
  }
  if(TAU::Parameters::paramType("ev") !=
     TAU::Parameters::Undef) {
    ev_str_ = TAU::Parameters::getString("ev");
  } else {
    IMP_THROW("Can't find ev parameters", ValueException);
  }

}

bool AlignmentParams::parse_parameters_strings() {
  if(!fitting_params_.add(fitting_str_)) {
    std::cerr << "Error parsing fitting options" << std::endl;
    return false;
  }
  if(!complementarity_params_.add(complementarity_str_)) {
    std::cerr << "Error parsing complementarity options" << std::endl;
    return false;
  }
  if(!domino_params_.add(domino_str_)) {
    std::cerr << "Error parsing domino options" << std::endl;
    return false;
  }
  if(!xlink_params_.add(xlink_str_)) {
    std::cerr << "Error parsing xlink options" << std::endl;
    return false;
  }
  if(!conn_params_.add(conn_str_)) {
    std::cerr << "Error parsing conn options" << std::endl;
    return false;
  }
  if(!rog_params_.add(rog_str_)) {
    std::cerr << "Error parsing rog options" << std::endl;
    return false;
  }
  if(!filters_params_.add(filters_str_)) {
    std::cerr << "Error parsing filters options" << std::endl;
    return false;
  }
  if(!ev_params_.add(ev_str_)) {
    std::cerr << "Error parsing ev options" << std::endl;
    return false;
  }
  if(!fragments_params_.add(fragments_str_)) {
    std::cerr << "Error parsing fragments options" << std::endl;
    return false;
  }
  return true;
}

IMPMULTIFIT_END_NAMESPACE
