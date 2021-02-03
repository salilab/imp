/**
 *  \file directories.h
 *  \brief Get directories used by IMP.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_DIRECTORIES_H
#define IMPKERNEL_INTERNAL_DIRECTORIES_H

#include <IMP/kernel_config.h>
#include <string>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
IMPKERNELEXPORT std::string get_concatenated_path(std::string part0,
                                                std::string part1);

IMPKERNELEXPORT std::string get_directory_path(std::string fileordirectory);

//! Get the path to a module data file.
IMPKERNELEXPORT std::string get_data_path(std::string module_name,
                                        std::string file_name);

//! Get the path to an example data file.
IMPKERNELEXPORT std::string get_example_path(std::string module_name,
                                           std::string file_name);

//! If data is not found in a normal location, it is searched for here
/** The module part of the expected path is not used. Use of this
    requires boost file system.
 */
IMPKERNELEXPORT void set_backup_data_path(std::string path);

IMPKERNELEXPORT std::string get_file_name(std::string path);

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_DIRECTORIES_H */
