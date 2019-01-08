/**
 *  \file alignment.cpp
 *  \brief Alignment of structures
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/alignment.h>
#include <IMP/algebra/geometric_alignment.h>

IMPATOM_BEGIN_NAMESPACE

algebra::Transformation3D get_transformation_aligning_first_to_second(
                             const Selection &s1, const Selection &s2)
{
  Particles ps1 = s1.get_selected_particles();
  Particles ps2 = s2.get_selected_particles();

  core::XYZs ds1(ps1.size());
  core::XYZs ds2(ps2.size());

  for (unsigned int i = 0; i < ps1.size(); ++i) {
    ds1[i] = core::XYZ(ps1[i]);
  }
  for (unsigned int i = 0; i < ps2.size(); ++i) {
    ds2[i] = core::XYZ(ps2[i]);
  }

  return algebra::get_transformation_aligning_first_to_second(ds1, ds2);
}

IMPATOM_END_NAMESPACE
