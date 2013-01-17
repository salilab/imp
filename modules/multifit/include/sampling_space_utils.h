/**
 *  \file sampling_space_utils.h
 *  \brief setting up tools
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_SAMPLING_SPACE_UTILS_H
#define IMPMULTIFIT_SAMPLING_SPACE_UTILS_H

#if 0

#include <IMP/multifit/DataContainer.h>
#include <IMP/domino1/TransformationCartesianProductSampler.h>
#include <IMP/multifit/multifit_config.h>
#include <IMP/multifit/multifit_config.h>
#include <IMP/domino1/TransformationCartesianProductSampler.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Set sampling space
/**
\param[in] data the system data
\return the sampling space
 */
IMPMULTIFITEXPORT IMP::domino1::TransformationCartesianProductSampler *
set_sampling_space(const DataContainer *data);

IMPMULTIFIT_END_NAMESPACE

#endif

#endif  /* IMPMULTIFIT_SAMPLING_SPACE_UTILS_H */
