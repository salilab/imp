/**
 *  \file RMF/decorator_utility.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_DECORATOR_UTILITY_H
#define IMPLIBRMF_DECORATOR_UTILITY_H

#include "RMF_config.h"
#include "NodeConstHandle.h"
#include "decorators.h"
namespace RMF {

  /** Return a list of Particle NodeHandles that forms a slice through the
      tree and whose radii are as close as possible to the passed resolution.
  */
  RMFEXPORT NodeConstHandles get_particles_by_resolution(NodeConstHandle h,
                                                         double resolution,
                                                         int frame=0);

} /* namespace RMF */

#endif /* IMPLIBRMF_DECORATOR_UTILITY_H */
