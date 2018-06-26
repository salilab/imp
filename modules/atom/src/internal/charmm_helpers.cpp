/**
 *  \file charmm_helpers.cpp
 *  \brief Helper functions for CHARMM support.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/CHARMMParameters.h>
#include <IMP/atom/CHARMMAtom.h>
#include <IMP/atom/angle_decorators.h>
#include <IMP/atom/internal/charmm_helpers.h>
#include <IMP/constants.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

void add_dihedral_to_list(const CHARMMParameters *param, Particle *p1,
                          Particle *p2, Particle *p3,
                          Particle *p4, Particles &ps) {
  if (CHARMMAtom::get_is_setup(p1) && CHARMMAtom::get_is_setup(p2)
      && CHARMMAtom::get_is_setup(p3) && CHARMMAtom::get_is_setup(p4)) {
    try {
      Vector<CHARMMDihedralParameters> p = param->get_dihedral_parameters(
          CHARMMAtom(p1).get_charmm_type(), CHARMMAtom(p2).get_charmm_type(),
          CHARMMAtom(p3).get_charmm_type(), CHARMMAtom(p4).get_charmm_type());
      for (Vector<CHARMMDihedralParameters>::const_iterator it = p.begin();
           it != p.end(); ++it) {
        Dihedral dd = Dihedral::setup_particle(
            new Particle(p1->get_model()), core::XYZ(p1), core::XYZ(p2),
            core::XYZ(p3), core::XYZ(p4));
        dd.set_ideal(it->ideal / 180.0 * PI);
        dd.set_multiplicity(it->multiplicity);
        if (it->force_constant < 0.0) {
          dd.set_stiffness(-std::sqrt(-it->force_constant * 2.0));
        } else {
          dd.set_stiffness(std::sqrt(it->force_constant * 2.0));
        }
        ps.push_back(dd);
      }
    }
    catch (const IndexException &e) {
      // If no parameters, warn, and create an empty dihedral
      IMP_WARN(e.what() << std::endl);
      Dihedral dd = Dihedral::setup_particle(
          new Particle(p1->get_model()), core::XYZ(p1), core::XYZ(p2),
          core::XYZ(p3), core::XYZ(p4));
      ps.push_back(dd);
    }
  } else {
    IMP_WARN("Missing CHARMM atom types for dihedral between "
             << p1->get_name() << ", " << p2->get_name() << ", "
             << p3->get_name() << " and " << p4->get_name() << std::endl);
    Dihedral dd = Dihedral::setup_particle(
          new Particle(p1->get_model()), core::XYZ(p1), core::XYZ(p2),
          core::XYZ(p3), core::XYZ(p4));
    ps.push_back(dd);
  }
}

IMPATOM_END_INTERNAL_NAMESPACE
