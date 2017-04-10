/**
 *  \file RMF/validate.h
 *  \brief Support for validating the data in a file.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_VALIDATE_H
#define RMF_VALIDATE_H

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "FileHandle.h"
#include "RMF/FileConstHandle.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/config.h"

RMF_ENABLE_WARNINGS

namespace RMF {
//! Validate the hierarchy, throwing an exception if any problems were found.
/** \throw exception if it is not valid and log any errors encountered.

    Checks invariants, such as all representation leaves having
    coordinates and radii.
 */
RMFEXPORT void validate(FileConstHandle fh);

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_VALIDATE_H */
