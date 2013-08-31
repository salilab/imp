/**
 *  \file IMP/example/counting.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_COUNTING_H
#define IMPEXAMPLE_COUNTING_H

#include <IMP/example/example_config.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/core/XYZ.h>

IMPEXAMPLE_BEGIN_NAMESPACE

/** Return the number of times particles from one set are close
    to those from another set.
    \note This method uses the distance between the centers of
    the particles and does not use their radii.
*/
inline unsigned int get_number_of_incidences(const kernel::ParticlesTemp &psa,
                                             const kernel::ParticlesTemp &psb,
                                             double point_distance) {
  algebra::Vector3Ds vsa(psa.size());
  for (unsigned int i = 0; i < vsa.size(); ++i) {
    vsa[i] = core::XYZ(psa[i]).get_coordinates();
  }
  IMP_NEW(algebra::NearestNeighbor3D, nn, (vsa));
  unsigned int ret = 0;
  for (unsigned int i = 0; i < psb.size(); ++i) {
    algebra::Vector3D v = core::XYZ(psb[i]).get_coordinates();
    if (!nn->get_in_ball(v, point_distance).empty()) {
      ++ret;
    }
  }
  return ret;
}

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_COUNTING_H */
