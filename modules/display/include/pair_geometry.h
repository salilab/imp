/**
 *  \file pair_geometry.h
 *  \brief Represent an XYZR particle with a sphere
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_PAIR_GEOMETRY_H
#define IMPDISPLAY_PAIR_GEOMETRY_H

#include "config.h"
#include "macros.h"

#include <IMP/PairContainer.h>
#include <IMP/core/XYZ.h>
#include <IMP/display/geometry.h>
#include <IMP/PairScore.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent a pair of particles with segment
/** A pair is represented by a segment with a given, constant
    radius. The name is taken from the name of the pair particle.
    \untested{PairGeometry}
 */
class IMPDISPLAYEXPORT PairGeometry: public Geometry
{
  core::XYZ d0_, d1_;
  Float radius_;
public:
  PairGeometry(core::XYZ d0, core::XYZ d1,
               Float radius=0);

  std::string get_name() const {
    return d0_.get_particle()->get_name();
  }

  IMP_GEOMETRY(PairGeometry, get_module_version_info());

};


//! Represent repesent a set of pairs of particles with segments
/** If a scoring function is passed it will display the derivatives
    for that function and, if threshold is set, only show pairs
    for which the function exceeds the threshold.

    \see PairGeometry
    \untested{PairsGeometries}
 */
class IMPDISPLAYEXPORT PairsGeometry: public CompoundGeometry
{
  IMP::internal::OwnerPointer<PairScore> f_;
  IMP::internal::OwnerPointer<PairContainer> c_;
  double threshold_;
  double radius_;
public:
  PairsGeometry(PairContainer *pc,
               Float radius=0);

  PairsGeometry(PairScore *f, PairContainer *c);

  void set_radius(double r){
    radius_=r;
  }

  void set_threshold(double t) {
    threshold_=t;
  }

  IMP_COMPOUND_GEOMETRY(PairsGeometry, get_module_version_info())
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_PAIR_GEOMETRY_H */
