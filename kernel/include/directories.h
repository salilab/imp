/**
 *  \file directories.h
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_DIRECTORIES_H
#define IMP_DIRECTORIES_H

#include "config.h"
#include "base_types.h"

IMP_BEGIN_NAMESPACE

//! Get the directory in which IMP read-only data files are stored.
IMPEXPORT String get_data_directory();

IMP_END_NAMESPACE

#endif  /* IMP_DIRECTORIES_H */
