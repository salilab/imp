/**
 *  \file sampling_space_utils.h
 *  \brief setting up tools
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_SAMPLING_SPACE_UTILS_H
#define IMPMULTIFIT_SAMPLING_SPACE_UTILS_H

#include <IMP/multifit/DataContainer.h>
#include <IMP/domino/TransformationCartesianProductSampler.h>
#include "multifit_config.h"
#include "multifit_config.h"
#include <IMP/domino/TransformationCartesianProductSampler.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Set sampling space
/**
\param[in] data the system data
\return the sampling space
 */
IMPMULTIFITEXPORT IMP::domino::TransformationCartesianProductSampler *
set_sampling_space(const DataContainer &data);

IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_SAMPLING_SPACE_UTILS_H */
