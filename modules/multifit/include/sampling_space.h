/**
 *  \file sampling_space.h
 *  \brief handles settting sampling space for different mappings
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_SAMPLING_SPACE_H
#define IMPMULTIFIT_SAMPLING_SPACE_H

#if 0

#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/domino1/TransformationMappedDiscreteSet.h>
#include <IMP/domino1/TransformationCartesianProductSampler.h>
#include <IMP/multifit/multifit_config.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Set a mapped discrete sampling space
/**
/param[in] full_smpl_space the full sampling space
/param[in] anchor_points the assembly anchor points
/param[in] mapping the mapping of components to anchor points
/note the function creates a new sampling space composed of all transformations
of a component that are close enough to its mapped anchor point.
For more details check out Lasker, JMB, 2009
 */
IMPMULTIFITEXPORT domino1::TransformationMappedDiscreteSet*
 create_mapped_discrete_set(
  domino1::TransformationMappedDiscreteSet* full_smpl_space,
  const Particles &asmb_anchor_points, const Particles &components,
  const Ints &mapping, Float dist_t=10.);

IMPMULTIFITEXPORT domino1::TransformationCartesianProductSampler*
 create_mapped_sampling_space(
  domino1::TransformationMappedDiscreteSet* full_smpl_space,
  const Particles &asmb_anchor_points, const Particles &components,
  const Ints &mapping, Float dist_t=10.);

IMPMULTIFIT_END_NAMESPACE
#endif

#endif /* IMPMULTIFIT_SAMPLING_SPACE_H */
