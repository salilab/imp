/**
 *  \file IMP/kernel/input_output.h
 *  \brief Single variable function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INPUT_OUTPUT_H
#define IMPKERNEL_INPUT_OUTPUT_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "container_base.h"
#include "Particle.h"

IMPKERNEL_BEGIN_NAMESPACE

/** \name Inputs and outputs
    These methods recursively find all inputs or outputs of a given type.

    If you don't want recursive, use the non input/output variants.
    @{
*/
/** Return all the input particles for a given ModelObject.*/
IMPKERNELEXPORT ParticlesTemp get_input_particles(const ModelObjectsTemp &mos);

/** Return all the input particles for a given ModelObject.*/
IMPKERNELEXPORT ContainersTemp
get_input_containers(const ModelObjectsTemp &mos);

/** Return all the output particles for a given ModelObject.*/
IMPKERNELEXPORT ParticlesTemp
get_output_particles(const ModelObjectsTemp &mos);

/** Return all the output particles for a given ModelObject.*/
IMPKERNELEXPORT ContainersTemp
get_output_containers(const ModelObjectsTemp &mos);
/** @} */

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_INPUT_OUTPUT_H */
