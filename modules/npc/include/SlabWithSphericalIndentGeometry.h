/**
 *  \file SlabWithSphericalIndentGeometry.h
 *  \brief A geometry for displaying a wireframe model of the surface with a spherical cap indent.
 *
 *  Copyright 2007-2018.
 *
 * */

#ifndef IMPNPC_SLAB_WITH_SPHERICAL_INDENT_GEOMETRY_H
#define IMPNPC_SLAB_WITH_SPHERICAL_INDENT_GEOMETRY_H

#include "npc_config.h"
#include <IMP/display/geometry.h>
#include <IMP/display/display_macros.h>

IMPNPC_BEGIN_NAMESPACE

class IMPNPCEXPORT SlabWithSphericalIndentGeometry : public display::Geometry {
//! Slab with specified parameters for radius and depth of the spherical indent and a length parameter
double radius_, depth_, length_;

public:
SlabWithSphericalIndentGeometry(double radius, double depth, double length);

virtual IMP::display::Geometries get_components() const override;

IMP_OBJECT_METHODS(SlabWithSphericalIndentGeometry);

};

IMPNPC_END_NAMESPACE

#endif  /* IMPNPC_SLAB_WITH_SPHERICAL_INDENT_GEOMETRY_H */