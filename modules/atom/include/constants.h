/**
 * \file IMP/atom/constants.h \brief Define the elements used in \imp.
 *
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_CONSTANTS_H
#define IMPATOM_CONSTANTS_H

#include <IMP/atom/atom_config.h>

IMPATOM_BEGIN_NAMESPACE
//! Return kT for a given temperature in units of [kcal/mol/K].
/** Value taken from
    \external{http://en.wikipedia.org/wiki/Boltzmann_constant,Wikipedia}.
*/
inline double get_kt(double T) { return 0.0019872041 * T; }

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CONSTANTS_H */
