/**
 *  \file LoopStatisticalPairScore.cpp
 *  \brief Fiser/Melo loop modeling statistical potential
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/LoopStatisticalPairScore.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>

IMPATOM_BEGIN_NAMESPACE

namespace {
void add_loop_statistical_score_data(Atom atom) {
  int type;
  Residue rd = get_residue(atom);
  std::string atom_string = atom.get_atom_type().get_string();
  std::string residue_string = rd.get_residue_type().get_string();
  std::string score_type = residue_string + '_' + atom_string;
  if (!LoopStatisticalType::get_key_exists(score_type)) {
    type = -1;
  } else {
//   std::cout << "Type for " << atom << " is "
//    << LoopStatisticalType(score_type) << std::endl;
    type = LoopStatisticalType(score_type).get_index();
  }
  if (type == -1 && atom.get_element() != H) {
    IMP_LOG_TERSE("Failed to find type for " << atom << " " << rd << std::endl);
  }
  if (atom->has_attribute(
       score_functor::LoopStatistical::get_loop_type_key())) {
    IMP_USAGE_CHECK(
     atom->get_value(
       score_functor::LoopStatistical::get_loop_type_key()) == type,
        "Atom " << atom << " already has loop_statistical score type "
                << "but it is not correct. Got "
                << atom->get_value(
                        score_functor::LoopStatistical::get_loop_type_key())
                << " expected " << type);
  } else {
    atom->add_attribute(
       score_functor::LoopStatistical::get_loop_type_key(), type);
  }
}
}

void add_loop_statistical_score_data(Hierarchy h) {
  Hierarchies atoms = get_by_type(h, ATOM_TYPE);
  for (unsigned int i = 0; i < atoms.size(); ++i) {
    add_loop_statistical_score_data(Atom(atoms[i]));
  }
}

IMPATOM_END_NAMESPACE
