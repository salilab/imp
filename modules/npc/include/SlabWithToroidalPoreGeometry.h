/**
 *  \file SlabWithToroidalPoreGeometry.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPNPC_SLAB_WITH_TOROIDAL_PORE_GEOMETRY_H
#define IMPNPC_SLAB_WITH_TOROIDAL_PORE_GEOMETRY_H

#include "npc_config.h"
#include <IMP/display/geometry.h>
#include <IMP/display/display_macros.h>

IMPNPC_BEGIN_NAMESPACE

//! XXXX
/** XXXXXX.
 */
class IMPNPCEXPORT SlabWithToroidalPoreWireGeometry : public display::Geometry {
  double rv_; // minor radius in vertical direction (vertical semi-axis)
  double rh_; // minor radius in horizontal direction (horizontal semi-axis)
  double R_; // major radius
  double slab_length_; // length of slab edge

 public:
  //! Slab with specified height from top to bottom, slab_length x slab_length area,
  //! and an elliptic toroidal pore of specified major radius,
  //! slab_height/2.0 vertical semi-axis, specified horizontal semi-axis
  SlabWithToroidalPoreWireGeometry(double slab_height,
                                   double major_radius,
                                   double horizontal_semiaxis,
                                   double slab_length);

  //! Slab with specified height from top to bottom, slab_length x slab_length area,
  //! and a ring toroidal pore of specified major radius and slab_height/2.0 minor radius
  SlabWithToroidalPoreWireGeometry(double slab_height, double major_radius, double slab_length);

  //! returns the set of geometric components that comprise this geometry
  //! (for e.g. storing in RMF format)
  virtual IMP::display::Geometries get_components() const override;

  IMP_OBJECT_METHODS(SlabWithToroidalPoreWireGeometry);
};


IMPNPC_END_NAMESPACE

#endif /* IMPNPC_SLAB_WITH_TOROIDAL_PORE_GEOMETRY_H */
