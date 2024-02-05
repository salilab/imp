/**
 *  \file SlabWithSphericalIndentGeometry.h
 *  \brief A geometry for displaying a wireframe model of the surface with a spherical cap indent.
 *
 *  Copyright 2007-2018.
 *
 * */

#ifndef IMPNPCASSEMBLY_SLAB_WITH_SPHERICAL_INDENT_GEOMETRY_H
#define IMPNPCASSEMBLY_SLAB_WITH_SPHERICAL_INDENT_GEOMETRY_H

#include "npcassembly_config.h"
#include <IMP/display/geometry.h>
#include <IMP/display/display_macros.h>

IMPNPCASSEMBLY_BEGIN_NAMESPACE

class IMPNPCASSEMBLYEXPORT SlabWithSphericalIndentGeometry : public display::Geometry {
//! Slab with specified parameters for radius and depth of the spherical indent and a length parameter
double radius_, depth_, length_;

public:
SlabWithSphericalIndentGeometry(double radius, double depth, double length);

virtual IMP::display::Geometries get_components() const IMP_OVERRIDE;

IMP_OBJECT_METHODS(SlabWithSphericalIndentGeometry);

};

IMPNPCASSEMBLY_END_NAMESPACE

#endif  /* IMPNPCASSEMBLY_SLAB_WITH_SPHERICAL_INDENT_GEOMETRY_H */