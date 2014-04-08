/**
 *  \file IMP/multifit/path_reader_writer.h
 *  \brief Read and write paths.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_PATH_READER_WRITER_H
#define IMPMULTIFIT_PATH_READER_WRITER_H

#include <IMP/multifit/multifit_config.h>
#include <IMP/domino/DominoSampler.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Read paths
/**
\note Notice that for the function to read the last line it should end with \n
 */
IMPMULTIFITEXPORT
IntsList read_paths(const char *txt_filename, int max_paths = INT_MAX);

IMPMULTIFITEXPORT
void write_paths(const IntsList &paths, const std::string &txt_filename);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_PATH_READER_WRITER_H */
