/**
 *  \file fitting_solutions_reader_writer.h
 *  \brief handles reading and writing MultiFit fitting solutions file
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FITTING_SOLUTIONS_READER_WRITER_H
#define IMPMULTIFIT_FITTING_SOLUTIONS_READER_WRITER_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/Object.h>
#include <IMP/file.h>
#include "multifit_config.h"
#include "FittingSolutionRecord.h"
IMPMULTIFIT_BEGIN_NAMESPACE

//! Fitting solutions reader
/**
\todo consider using TextInput
 */
IMPMULTIFITEXPORT FittingSolutionRecords read_fitting_solutions(
  const char *fitting_fn);

IMPMULTIFITEXPORT void  write_fitting_solutions(
   const char *fitting_fn,
   const FittingSolutionRecords &fit_sols);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_FITTING_SOLUTIONS_READER_WRITER_H */
