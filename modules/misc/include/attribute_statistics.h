/**
 *  \file attribute_statistics.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_ATTRIBUTE_STATISTICS_H
#define IMPMISC_ATTRIBUTE_STATISTICS_H

#include "config.h"

#include <IMP/Model.h>

IMPMISC_BEGIN_NAMESPACE

//! Print statistics about the attributes used in Particles.
/** For each type of attribute, the function prints the names of all
    the attributes and a list of how many particles has each pattern
    of attributes. Since the attributes are currently stored in
    arrays, a pattern is printed as a "-" if the particle doesn't have
    that attribute and a "X" if it does.
 */
IMPMISCEXPORT void show_attribute_usage(Model *m, std::ostream &out= std::cout);


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_ATTRIBUTE_STATISTICS_H */
