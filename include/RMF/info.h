/**
 *  \file RMF/info.h
 *  \brief Show info about the file.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INFO_H
#define RMF_INFO_H

#include "RMF/config.h"
#include "FileConstHandle.h"
#include <iostream>

RMF_ENABLE_WARNINGS

namespace RMF {
/** Show various statistics about the file such as attributes
    and how often they are used. */
RMFEXPORT void show_info(RMF::FileConstHandle rh,
                         std::ostream& out = std::cout);

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INFO_H */
