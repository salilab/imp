/**
 *  \file display/internal/utility.h
 *  \brief Base class for writing geometry to a file
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_INTERNAL_UTILITY_H
#define IMPDISPLAY_INTERNAL_UTILITY_H

#include "../display_config.h"
#include <IMP/algebra/Vector3D.h>

IMPDISPLAY_BEGIN_INTERNAL_NAMESPACE

IMPDISPLAYEXPORT
std::pair<std::vector<algebra::Vector3Ds>, algebra::Vector3D>
get_convex_polygons(const algebra::Vector3Ds &poly);

IMPDISPLAY_END_INTERNAL_NAMESPACE

#endif  /* IMPDISPLAY_INTERNAL_UTILITY_H */
