/**
 *  \file IMP/multifit/fitting_utils.h
 *  \brief fitting utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FITTING_UTILS_H
#define IMPMULTIFIT_FITTING_UTILS_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/kernel/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/base/Object.h>
#include <IMP/base/file.h>
#include <IMP/em/rigid_fitting.h>
#include <IMP/multifit/multifit_config.h>
#include "FittingSolutionRecord.h"
IMPMULTIFIT_BEGIN_NAMESPACE

//create a density hit map from a set of fitting solutions
IMPMULTIFITEXPORT
em::DensityMap* create_hit_map(core::RigidBody rb, Refiner *rb_ref,
                            const FittingSolutionRecords &sols,
                            em::DensityMap *damp
                            );
//! Given a molecule and a set of points, return the indexes of the points
//! that are close to the molecule (up to max_dist) and the res
IMPMULTIFITEXPORT
algebra::Vector3Ds get_points_close_to_molecule(
                        const atom::Hierarchy &mh,
                        const algebra::Vector3Ds points,
                        Float max_dist);
IMPMULTIFITEXPORT
algebra::Vector3Ds get_points_far_from_molecule(
                        const atom::Hierarchy &mh,
                        const algebra::Vector3Ds points,
                        Float max_dist);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_FITTING_UTILS_H */
