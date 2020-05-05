/**
 *  \file RMF/names.h
 *  \brief Functions to check if names are valid.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
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
/** Node and key names have to obey certain rules, such as no quotes
    in the name. */
	
//! Return a node name string that has been modified to obey the rules.
RMFEXPORT std::string get_as_node_name(std::string input);

//! Return true iff the string is a valid key name.
RMFEXPORT bool get_is_valid_key_name(std::string name);

//! Return true iff the string is a valid node name.
RMFEXPORT bool get_is_valid_node_name(std::string name);

/** @} */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_NAMES_H */
