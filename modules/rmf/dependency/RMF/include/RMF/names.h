/**
 *  \file RMF/names.h
 *  \brief Functions to check if names are valid.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_NAMES_H
#define RMF_NAMES_H

#include <sstream>
#include <string>

#include "RMF/config.h"
#include "types.h"

RMF_ENABLE_WARNINGS
namespace RMF {

/** \name Names
    @{ */
/** Node names have to obey certain rules, such as no quotes
    in the name. This returns a string that has been modified to
    obey the rules.*/
RMFEXPORT std::string get_as_node_name(std::string input);

RMFEXPORT bool get_is_valid_key_name(std::string name);
RMFEXPORT bool get_is_valid_node_name(std::string name);

/** @} */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_NAMES_H */
