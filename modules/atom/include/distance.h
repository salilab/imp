/**
 *  \file distance.h
 *  \brief distance metrics
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_DISTANCE_H
#define IMPATOM_DISTANCE_H

#include "config.h"
#include <IMP/core/XYZ.h>
#include "Hierarchy.h"

IMPATOM_BEGIN_NAMESPACE
//! Calculate the root mean square deviation between two sets of 3D points.
/**
\note the function assumes correspondence between two sets of points and does
not perform rigid alignment.
 */
IMPATOMEXPORT double rmsd(const core::XYZs& m1 ,const core::XYZs& m2);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DISTANCE_H */
