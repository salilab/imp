/**
 * \file atom/CHARMMParameters.h \brief access to Charmm force field parameters
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_CHARMM_PARAMETERS_H
#define IMPATOM_CHARMM_PARAMETERS_H

#include "internal/charmm_helpers.h"
#include "ForceFieldParameters.h"
#include "charmm_topology.h"
#include "macros.h"

#include <string>
#include <fstream>

// swig is being dumb
IMP_BEGIN_NAMESPACE
class VersionInfo;
IMP_END_NAMESPACE

IMPATOM_BEGIN_NAMESPACE

struct CHARMMBondParameters {
  double force_constant;
  double ideal;
};

struct CHARMMDihedralParameters {
  double force_constant;
  int multiplicity;
  double ideal;
};

//! Charmm force field
class IMPATOMEXPORT CHARMMParameters : public ForceFieldParameters {
  std::map<std::string, CHARMMIdealResidueTopology> residue_topologies_;
  std::map<std::string, CHARMMPatch> patches_;
  std::map<internal::CHARMMBondNames, CHARMMBondParameters> bond_parameters_;
  std::map<internal::CHARMMAngleNames, CHARMMBondParameters> angle_parameters_;

  typedef std::vector<std::pair<internal::CHARMMDihedralNames,
                                CHARMMDihedralParameters> > DihedralParameters;
  DihedralParameters dihedral_parameters_;
  DihedralParameters improper_parameters_;

  DihedralParameters::const_iterator
        find_dihedral(DihedralParameters::const_iterator begin,
                      DihedralParameters::const_iterator end,
                      const internal::CHARMMDihedralNames &dihedral,
                      bool allow_wildcards) const;

public:

  /** construction with Charmm parameters file
      for addition of bonds topology file is enough,
      for the rest both files are needed
   */
  CHARMMParameters(const String& topology_file_name,
                   const String& par_file_name = std::string());

  void add_patch(CHARMMPatch &patch) {
    patches_.insert(std::make_pair(patch.get_type(), patch));
  }

  void add_residue_topology(CHARMMIdealResidueTopology &res) {
    residue_topologies_.insert(std::make_pair(res.get_type(), res));
  }

  CHARMMPatch &get_patch(std::string name) {
    std::map<std::string, CHARMMPatch>::iterator it = patches_.find(name);
    if (it != patches_.end()) {
      return it->second;
    } else {
      IMP_THROW("Patch " << name << " does not exist", ValueException);
    }
  }

#ifndef SWIG
  const CHARMMPatch &get_patch(std::string name) const {
    std::map<std::string, CHARMMPatch>::const_iterator it = patches_.find(name);
    if (it != patches_.end()) {
      return it->second;
    } else {
      IMP_THROW("Patch " << name << " does not exist", ValueException);
    }
  }
#endif

  CHARMMIdealResidueTopology &get_residue_topology(std::string name) {
    std::map<std::string, CHARMMIdealResidueTopology>::iterator it
              = residue_topologies_.find(name);
    if (it != residue_topologies_.end()) {
      return it->second;
    } else {
      IMP_THROW("Residue " << name << " does not exist", ValueException);
    }
  }

#ifndef SWIG
  const CHARMMIdealResidueTopology &get_residue_topology(std::string name) const
  {
    std::map<std::string, CHARMMIdealResidueTopology>::const_iterator it
              = residue_topologies_.find(name);
    if (it != residue_topologies_.end()) {
      return it->second;
    } else {
      IMP_THROW("Residue " << name << " does not exist", ValueException);
    }
  }
#endif

  CHARMMTopology *create_topology(Hierarchy hierarchy) const;

  const CHARMMBondParameters *get_bond_parameters(std::string type1,
                                                  std::string type2) const {
    internal::CHARMMBondNames types = internal::CHARMMBondNames(type1, type2);
    if (bond_parameters_.find(types) != bond_parameters_.end()) {
      return &bond_parameters_.find(types)->second;
    } else {
      return NULL;
    }
  }

  const CHARMMBondParameters *get_angle_parameters(std::string type1,
                                                   std::string type2,
                                                   std::string type3) const {
    internal::CHARMMAngleNames types = internal::CHARMMAngleNames(type1, type2,
                                                                  type3);
    if (angle_parameters_.find(types) != angle_parameters_.end()) {
      return &angle_parameters_.find(types)->second;
    } else {
      return NULL;
    }
  }

  std::vector<CHARMMDihedralParameters> get_dihedral_parameters(
             std::string type1, std::string type2, std::string type3,
             std::string type4) const {
    std::vector<CHARMMDihedralParameters> param;
    internal::CHARMMDihedralNames types = internal::CHARMMDihedralNames(
                    type1, type2, type3, type4);
    // Get the first match, using wildcards
    DihedralParameters::const_iterator match =
        find_dihedral(dihedral_parameters_.begin(),
                      dihedral_parameters_.end(), types, true);
    if (match != dihedral_parameters_.end()) {
      // If it matched, look for duplicate dihedral terms (this time the
      // match must be exactly the same as the first match)
      param.push_back(match->second);
      while ((match = find_dihedral(match + 1, dihedral_parameters_.end(),
                                    match->first, false))
             != dihedral_parameters_.end()) {
        param.push_back(match->second);
      }
    }
    return param;
  }

  const CHARMMDihedralParameters *get_improper_parameters(
             std::string type1, std::string type2, std::string type3,
             std::string type4) const {
    internal::CHARMMDihedralNames types = internal::CHARMMDihedralNames(
                    type1, type2, type3, type4);
    // Return just the first match; wildcards are OK
    DihedralParameters::const_iterator it =
        find_dihedral(improper_parameters_.begin(),
                      improper_parameters_.end(), types, true);
    if (it != improper_parameters_.end()) {
      return &it->second;
    } else {
      return NULL;
    }
  }

  IMP_FORCE_FIELD_PARAMETERS(CHARMMParameters);
private:

  virtual String get_force_field_atom_type(Atom atom) const;

  void read_parameter_file(std::ifstream& input_file);
  // read topology file
  void read_topology_file(std::ifstream& input_file);

  ResidueType parse_residue_line(const String& line);
  void parse_atom_line(const String& line, ResidueType curr_res_type,
                       CHARMMResidueTopologyBase &residue);
  void parse_bond_line(const String& line, ResidueType curr_res_type,
                       CHARMMResidueTopologyBase &residue);

  void parse_nonbonded_parameters_line(String line);
  void parse_bonds_parameters_line(String line);
  void parse_angles_parameters_line(String line);
  void parse_dihedrals_parameters_line(String line,
                                       DihedralParameters &param);
  WarningContext warn_context_;
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHARMM_PARAMETERS_H */
