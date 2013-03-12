/**
 *  \file RMF/decorator_utility.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_DECORATOR_UTILITY_H
#define RMF_DECORATOR_UTILITY_H

#include <RMF/config.h>
#include "NodeConstHandle.h"
#include <RMF/decorators.h>
RMF_ENABLE_WARNINGS
namespace RMF {

/** Return a list of Particle NodeHandles that forms a slice through the
    tree and whose radii are as close as possible to the passed resolution.
 */
RMFEXPORT NodeConstHandles get_particles_by_resolution(NodeConstHandle h,
                                                       double          resolution);

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_DECORATOR_UTILITY_H */
