/**
 *  \file LogOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_LOG_OPTIMIZER_STATE_H
#define IMPDISPLAY_LOG_OPTIMIZER_STATE_H

#include "config.h"
#include "Writer.h"
#include <IMP/OptimizerState.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/geometry.h>
#include <IMP/Pointer.h>
#include <IMP/FailureHandler.h>
#include <IMP/internal/utility.h>
#include <vector>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write geometry to a container periodically
/** The State writes a series of files generated from a
    printf-style format string. The added geometry objects
    are stored by reference internally.
    \verbinclude log.py
    \ingroup logging
 */
class IMPDISPLAYEXPORT LogOptimizerState: public OptimizerState
{
  mutable IMP::internal::OwnerPointer<Writer> writer_;
  std::string name_template_;

  CompoundGeometries edata_;
  Geometries gdata_;

public:
  //! Write files using name_template as a template (must have a %d in it)
  LogOptimizerState(Writer *writer, std::string name_template);

  void add_geometry(CompoundGeometry* g);

  void add_geometry(Geometry* g);

  void add_geometry(const CompoundGeometries& g);

  void add_geometry(const Geometries& g);

  //! Force writing the a file with the given name
  void write(std::string file_name) const;

  IMP_PERIODIC_OPTIMIZER_STATE(LogOptimizerState,
                               get_module_version_info())
};



/** \brief Dump the state of the model to a file on an error and then
    go on the the other handlers.

    When an error (check or assertion failure) occurs, the model is
    dumped to the specified using the geometry given in the
    LogOptimizerState.
 */
class IMPDISPLAYEXPORT DisplayModelOnFailure: public FailureHandler {
  IMP::internal::OwnerPointer<LogOptimizerState> s_;
  std::string file_name_;
 public:
  DisplayModelOnFailure(LogOptimizerState *m, std::string file_name);
  IMP_FAILURE_HANDLER(DisplayModelOnFailure, get_module_version_info());
};

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_LOG_OPTIMIZER_STATE_H */
