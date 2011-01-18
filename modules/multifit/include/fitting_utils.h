/**
 *  \file fitting_utils.h
 *  \brief fitting utilities
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FITTING_UTILS_H
#define IMPMULTIFIT_FITTING_UTILS_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/Object.h>
#include <IMP/file.h>
#include <IMP/em/rigid_fitting.h>
#include "multifit_config.h"
#include "FittingSolutionRecord.h"
IMPMULTIFIT_BEGIN_NAMESPACE

//create a density hit map from a set of fitting solutions
IMPMULTIFITEXPORT
em::DensityMap* create_hit_map(core::RigidBody &rb, Refiner *rb_ref,
                            const FittingSolutionRecords &sols,
                            em::DensityMap *damp
                            );


IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_FITTING_UTILS_H */
