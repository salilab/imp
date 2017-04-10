/**
 *  \file RMF/signature.h
 *  \brief Return a (long) string describing a file that can be compared.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_SIGNATURE_H
#define RMF_SIGNATURE_H

#include "RMF/config.h"
#include "FileConstHandle.h"

RMF_ENABLE_WARNINGS

namespace RMF {

//! Return a string that captures information about the file.
/** This string can be saved and compared against the string generated
    with future versions of the library to make sure that the old files
    parse reasonably well. The strings are diffable. */
RMFEXPORT std::string get_signature_string(FileConstHandle file);

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_SIGNATURE_H */
