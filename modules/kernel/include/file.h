/**
 *  \file IMP/kernel/file.h
 *  \brief For backwards compatibility.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_FILE_H
#define IMPKERNEL_FILE_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/file.h>
#include <fstream>
#include <iostream>

IMPKERNEL_BEGIN_NAMESPACE
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
//swig can't deal with things
//using base::TextInput;
//using base::TextOutput;
using IMP::base::create_temporary_file;
using IMP::base::create_temporary_file_name;
using IMP::base::set_log_target;
#endif
IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_FILE_H */
