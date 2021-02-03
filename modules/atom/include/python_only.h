/**
 *  \file IMP/atom/python_only.h
 *  \brief Functionality only available in Python.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_PYTHON_ONLY_H
#define IMPATOM_PYTHON_ONLY_H

#include <IMP/atom/atom_config.h>

IMPATOM_BEGIN_NAMESPACE

/** \name Python Only
    The following functions are only available in Python as the
    equivalent C++ functionality is provided via template
    functions or in other ways that don't directly map to
    Python.
    @{
*/
#ifdef IMP_DOXYGEN

//! Print out the molecular hierarchy.
/** Equivalent to
\code
IMP::core::show(h);
\endcode
*/
void show_molecular_hierarchy(Hierarchy h);
#endif
/** @} */

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PYTHON_ONLY_H */
