/**
 *  \file RMF/numpy_util.h
 *  \brief Utility functions to access data from Python with numpy.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_NUMPY_UTIL_H
#define RMF_NUMPY_UTIL_H

#include "RMF/config.h"

namespace RMF {

// We only document the functions here. The implementations are provided
// in swig/RMF.numpy.i

#ifdef RMF_DOXYGEN

//! Get global XYZ coordinates for everything under the given node.
/** This will traverse the subset of the RMF file rooted at the given node
    in a depth first manner and put the XYZ coordinates for all appropriate
    nodes into the provided NumPy array, which must be an N*3 float array
    where N is the number of XYZ particles. The coordinates are global, i.e.
    they are transformed by any parent ReferenceFrame nodes.

    This function is primarily intended to extract trajectory frames
    for visualization packages such as ChimeraX. It is only available to
    Python and if RMF is built with NumPy support.
  */
void get_all_global_coordinates(
     FileConstHandle &fh, NodeConstHandle &nh, PyObject *coord);

#endif

} /* namespace RMF */

#endif /* RMF_NUMPY_UTIL_H */
