/**
 *  \file utility.h
 *  \brief Miscellaneous utility functions.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCNMULTIFIT_UTILITY_H
#define IMPCNMULTIFIT_UTILITY_H

#include <IMP/algebra/Transformation3D.h>
#include <libTAU/RigidTrans3.h>
#include "../cnmultifit_config.h"

IMPCNMULTIFIT_BEGIN_INTERNAL_NAMESPACE

IMPCNMULTIFITEXPORT
algebra::Transformation3D tau2imp(const TAU::RigidTrans3 &t);

IMPCNMULTIFIT_END_INTERNAL_NAMESPACE

#endif  /* IMPCNMULTIFIT_UTILITY_H */
