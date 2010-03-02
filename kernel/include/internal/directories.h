/**
 *  \file directories.h
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_DIRECTORIES_H
#define IMP_INTERNAL_DIRECTORIES_H

#include "../config.h"
#include <string>

IMP_BEGIN_INTERNAL_NAMESPACE

//! Get the the path to a module data file.
IMPEXPORT std::string get_data_path(std::string module_name,
                                    std::string file_name);



//! Get the path to an example data file.
IMPEXPORT std::string get_example_path(std::string module_name,
                                       std::string file_name);

//! If data is not found in a normal location, it is searched for here
/** The module part of the expected path is not used. Use of this
    requires boost file system.
 */
IMPEXPORT void set_backup_data_path(std::string path) ;

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_DIRECTORIES_H */
