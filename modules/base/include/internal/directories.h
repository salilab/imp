/**
 *  \file directories.h
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_INTERNAL_DIRECTORIES_H
#define IMPBASE_INTERNAL_DIRECTORIES_H

#include <IMP/base/base_config.h>
#include <string>

IMPBASE_BEGIN_INTERNAL_NAMESPACE IMPBASEEXPORT std::string
get_concatenated_path(std::string part0, std::string part1);

IMPBASEEXPORT std::string get_directory_path(std::string fileordirectory);

//! Get the the path to a module data file.
IMPBASEEXPORT std::string get_data_path(std::string module_name,
                                        std::string file_name);

//! Get the path to an example data file.
IMPBASEEXPORT std::string get_example_path(std::string module_name,
                                           std::string file_name);

//! If data is not found in a normal location, it is searched for here
/** The module part of the expected path is not used. Use of this
    requires boost file system.
 */
IMPBASEEXPORT void set_backup_data_path(std::string path);

IMPBASEEXPORT std::string get_file_name(std::string path);

IMPBASE_END_INTERNAL_NAMESPACE

#endif /* IMPBASE_INTERNAL_DIRECTORIES_H */
