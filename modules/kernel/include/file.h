/**
 *  \file IMP/file.h
 *  \brief Handling of file input/output
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_FILE_H
#define IMPKERNEL_FILE_H

#include "kernel_config.h"
#include <IMP/base/file.h>
#include <fstream>
#include <iostream>

IMP_BEGIN_NAMESPACE
#ifndef SWIG
using base::TextInput;
using base::TextOutput;
using IMP::base::create_temporary_file;
using IMP::base::create_temporary_file_name;
#endif
IMP_END_NAMESPACE

#endif /* IMPKERNEL_FILE_H */
