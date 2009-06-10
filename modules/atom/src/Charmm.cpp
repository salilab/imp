/**
 * \file Charmm \brief access to Charmm force field parameters
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/Charmm.h>
#include <IMP/core/XYZR.h>
#include <IMP/Particle.h>

#include <boost/algorithm/string.hpp>

IMPATOM_BEGIN_NAMESPACE

Charmm::Charmm(const String& par_file_name, const String& topology_file_name) :
  topology_(topology_file_name)
{
  std::ifstream par_file(par_file_name.c_str());
  if(!par_file) {
    std::cerr << "Can't open charmm file " << par_file_name << std::endl;
    exit(1);
  }
  read_VdW_params(par_file);
  par_file.close();
}

Float Charmm::get_radius(const AtomType& atom_type,
                         const ResidueType& residue_type) const
{
  String charmm_atom_type =
    topology_.get_charmm_atom_type(atom_type, residue_type);
  return get_radius(charmm_atom_type);
}

Float Charmm::get_epsilon(const AtomType& atom_type,
                          const ResidueType& residue_type) const
{
  String charmm_atom_type =
    topology_.get_charmm_atom_type(atom_type, residue_type);
  return get_epsilon(charmm_atom_type);
}

void Charmm::add_radius(Hierarchy mhd, FloatKey radius_key) const
{
  Particles ps = get_by_type(mhd, Hierarchy::ATOM);
  for(unsigned int i=0; i<ps.size(); i++) {
    Float radius = get_radius(Atom(ps[i]).get_atom_type(),
                              get_residue_type(Atom(ps[i])));
    core::XYZR::create(ps[i], radius, radius_key);
  }
}

Float Charmm::get_radius(const String& charmm_atom_type) const
{
  if(charmm_atom_type.length() > 0 &&
     charmm_2_vdW_.find(charmm_atom_type) != charmm_2_vdW_.end()) {
    return charmm_2_vdW_.find(charmm_atom_type)->second.second;
  }
  std::cerr << "Warning! Radius not found " << charmm_atom_type << std::endl;
  return 1.7; // SOME DEFAULT VALUE!!
}

Float Charmm::get_epsilon(const String& charmm_atom_type) const
{
  if(charmm_atom_type.length() > 0 &&
     charmm_2_vdW_.find(charmm_atom_type) != charmm_2_vdW_.end()) {
    return charmm_2_vdW_.find(charmm_atom_type)->second.first;
  }
  std::cerr << "Warning! Epsilon not found " << charmm_atom_type << std::endl;
  return -0.1; // SOME DEFAULT VALUE!!
}

void Charmm::read_VdW_params(std::ifstream& input_file) {
  const String NONBONDED_LINE = "NONBONDED";

  bool in_nonbonded = false;
  while (!input_file.eof()) {
    String line;
    getline(input_file, line);

    boost::trim(line); // remove all spaces
    // skip comments or empty lines
    if (line[0] == '!' || line[0] == '*' || line.length() == 0) continue;

    if(line.substr(0, NONBONDED_LINE.length()) == NONBONDED_LINE) {
      in_nonbonded=true;
      getline(input_file, line); //remove second line of NONBONDED
      continue;
    }

    if(line.substr(0, 5) == "HBOND" || line.substr(0, 3) == "END" ||
       line.substr(0, 5) == "NBFIX") break; // eof NONBONDED

    if(in_nonbonded) {
      std::vector<String> split_results;
      boost::split(split_results, line, boost::is_any_of(" "),
                   boost::token_compress_on);
      if(split_results.size() < 4)
        continue; // non-bonded line has at least 4 fields

      String charmm_atom_type = split_results[0];
      float epsilon = atof(split_results[2].c_str());
      float radius = atof(split_results[3].c_str());
      charmm_2_vdW_[charmm_atom_type] = std::make_pair(epsilon, radius);
    }
  }

  if(charmm_2_vdW_.size() == 0) {
    std::cerr << "NONBONDED params not found in Charmm parameter file"
              << std::endl;
    exit(1);
  }
}

IMPATOM_END_NAMESPACE
