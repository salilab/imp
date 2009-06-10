/**
 * \file Charmm \brief access to Charmm force field parameters
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_CHARMM_H
#define IMPATOM_CHARMM_H

#include "Residue.h"
#include "Atom.h"
#include "Hierarchy.h"
#include "internal/Topology.h"

#include <IMP/base_types.h>

#include <string>
#include <fstream>
#include <map>
#include <utility>

IMPATOM_BEGIN_NAMESPACE

//! Storage and access to Charmm force field
/** \unstable{Charmm}
 */
class Charmm {
public:

  //! construction with Charmm parameters file
  Charmm(const String& par_file_name,
         const String& topology_file_name);

  //! get radius
  Float get_radius(const AtomType& atom_type,
                   const ResidueType& residue_type) const;

  //! get epsilon for non bonded vdW
  Float get_epsilon(const AtomType& atom_type,
                    const ResidueType& residue_type) const;

  //! add radii to the structure defined in the hierarchy
  void add_radius(Hierarchy mhd, FloatKey radius_key= FloatKey("radius")) const;

private:

  Float get_radius(const String& charmm_atom_type) const;
  Float get_epsilon(const String& charmm_atom_type) const;

  // read non-bonded parameters for VdW computation
  void read_VdW_params(std::ifstream& input_file);

private:
  // map that holds charmm parameters according to charmm atom types
  // key=charmm_atom_type, value=(epsilon,radius)
  std::map<String, std::pair<float, float> > charmm_2_vdW_;

  internal::Topology topology_;
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHARMM_H */
