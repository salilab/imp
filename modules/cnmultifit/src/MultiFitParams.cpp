/**
 *  \file MultiFitParams.cpp
 *  \brief Parameters for MultiFit.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <IMP/cnmultifit/MultiFitParams.h>
#include <libTAU/Parameters.h>

IMPCNMULTIFIT_BEGIN_NAMESPACE

MultiFitParams::MultiFitParams(const char* paramFile) {
  std::ifstream params(paramFile);
  TAU::Parameters::readParameters(params);
}

bool MultiFitParams::processParameters() {
  getParameters();
  if(!parse_parameters_strings())
    return false;
  return true;
}

bool BaseParams2::add(const std::string str) {
  if(sscanf(str.c_str(), "%f %f", &min_base_dist, &max_base_dist) != 2)
    return false;
  min_base_dist = min_base_dist*min_base_dist;
  max_base_dist = max_base_dist*max_base_dist;
  return true;
}

bool ClusterParams2::add(const std::string str) {
  if(sscanf(str.c_str(), "%f %d %f", &maxAngleDiff, &ignoreClustersSmaller,
            &rmsd) != 3)
    return false;
  return true;
}

bool ScoreParams2::add(const std::string str) {
  if(sscanf(str.c_str(), "%f %f %f %d %d %d %d %d", &small_interface_ratio,
            &max_penetration, &ns_thr, &weights[0], &weights[1], &weights[2],
            &weights[3], &weights[4]) != 8)
    return false;
  return true;
}

bool SymmetryParams2::add(const std::string &str) {
  if(sscanf(str.c_str(), "%d", &cn) != 1)
    return false;
  return true;
}

bool ActiveSiteParams2::add(std::string str) {
  std::string s(str);
  active_site_filename = TAU::Parameters::nextToken(s);
  matching_use = atoi(TAU::Parameters::nextToken(s).c_str());
  if(!(matching_use == 0 || matching_use == 1 || matching_use ==2))
    return false;
  scoring_ratio = atof(TAU::Parameters::nextToken(s).c_str());
  if(scoring_ratio < 0.0 || scoring_ratio > 1.0)
    return false;
  return true;
}

void MultiFitParams::getParameters() {
  if(TAU::Parameters::paramType("monomer") != TAU::Parameters::Undef) {
    monomer_pdb_fn_ = TAU::Parameters::getString("monomer");
  } else {
    std::cerr << "Can't find monomer pdb  parameter" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("monomer_ms") != TAU::Parameters::Undef) {
    monomer_ms_fn_ = TAU::Parameters::getString("monomer_ms");
  } else {
    std::cerr << "Can't find monomer ms  parameter" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("refPdb") != TAU::Parameters::Undef) {
    ref_pdb_fn_ = TAU::Parameters::getString("refPdb");
  }
  if(TAU::Parameters::paramType("prot_lib") != TAU::Parameters::Undef) {
    prot_lib_ = TAU::Parameters::getString("prot_lib");
  } else {
    std::cerr << "Can't find protLib parameter" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("grid") != TAU::Parameters::Undef) {
    grid = TAU::Parameters::getString("grid");
  } else {
    std::cerr << "Can't find grid parameter" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("density") != TAU::Parameters::Undef) {
    density_map_fn_ = TAU::Parameters::getString("density");
  } else {
    std::cerr << "Can't find density parameter" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("density_resolution")
     != TAU::Parameters::Undef) {
    density_map_res_ = TAU::Parameters::getFloat("density_resolution");
  } else {
    std::cerr << "Can't find density resolution parameter" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("density_threshold")
     != TAU::Parameters::Undef) {
    density_thr_ = TAU::Parameters::getFloat("density_threshold");
  } else {
    std::cerr << "Can't find density threshold parameter" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("density_spacing") != TAU::Parameters::Undef) {
    density_spacing_ = TAU::Parameters::getFloat("density_spacing");
  } else {
    std::cerr << "Can't find density threshold parameter" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("baseParams") != TAU::Parameters::Undef)
    baseParams = TAU::Parameters::getString("baseParams");
  if(TAU::Parameters::paramType("scoreParams") != TAU::Parameters::Undef) {
    scoreParams = TAU::Parameters::getString("scoreParams");
  } else {
    std::cerr << "Can't find scoring parameters" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("symmetry_params") != TAU::Parameters::Undef) {
    symmetryParams = TAU::Parameters::getString("symmetry_params");
  } else {
    std::cerr << "Can't find symmetry parameters" << std::endl;
    exit(1);
  }
  if(TAU::Parameters::paramType("activeSiteParams") != TAU::Parameters::Undef)
    activeSiteParams = TAU::Parameters::getString("activeSiteParams");
  if(TAU::Parameters::paramType("clusterParams") != TAU::Parameters::Undef)
    clusterParams = TAU::Parameters::getString("clusterParams");
  return;
}

bool MultiFitParams::parse_parameters_strings() {
  if(!params.gridParams.add(grid)) {
    std::cerr << "Parsing error at grid option" << std::endl;
    return false;
  }
  if(!params.dockingParams.baseParams.add(baseParams)) {
      std::cerr << "Parsing error at baseParams option" << std::endl;
      return false;
  }
  if(!params.dockingParams.clusterParams.add(clusterParams)) {
    std::cerr << "Parsing error at clusterParams option" << std::endl;
    return false;
  }
  if(!params.dockingParams.scoreParams.add(scoreParams)) {
    std::cerr << "Parsing error at scoreParams option" << std::endl;
    return false;
  }
  if(!params.dockingParams.symmetryParams.add(symmetryParams)) {
    std::cerr << "Parsing error at symmetryParams option:"
              << symmetryParams << std::endl;
    return false;
  }
  if(!activeSiteParams.empty()
     && !params.dockingParams.activeSiteParams.add(activeSiteParams)) {
    std::cerr << "Parsing error at activeSiteParams option:"
              << activeSiteParams << std::endl;
    return false;
  }

  params.unitPdb = monomer_pdb_fn_;
  params.unitMs = monomer_ms_fn_;
  params.refPdb = ref_pdb_fn_;
  params.protLib = prot_lib_;
  return true;
}

IMPCNMULTIFIT_END_NAMESPACE
