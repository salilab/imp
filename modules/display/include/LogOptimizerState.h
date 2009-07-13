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
#include "internal/version_info.h"
#include <IMP/OptimizerState.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/geometry.h>
#include <IMP/Pointer.h>
#include <IMP/FailureHandler.h>
#include <vector>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write geometry to a container periodically
/** The State writes a series of files generated from a
    printf-style format string. The added geometry objects
    are stored by reference internally.
    \verbinclude log.py
    \ingroup log
 */
class IMPDISPLAYEXPORT LogOptimizerState: public OptimizerState
{
  mutable Pointer<Writer> writer_;
  unsigned int step_;
  unsigned int skip_steps_;
  std::string name_template_;

  CompoundGeometries edata_;
  Geometries gdata_;

public:
  //! Write files using name_template as a template (must have a %d in it)
  LogOptimizerState(Writer *writer, std::string name_template);

  //! Set the number of steps to skip
  void set_skip_steps(unsigned int i) {
    skip_steps_=i;
  }

  void add_geometry(CompoundGeometry* g);

  void add_geometry(Geometry* g);

  void add_geometry(const CompoundGeometries& g);

  void add_geometry(const Geometries& g);

  //! Force writing the a file with the given name
  void write(std::string file_name) const;

  IMP_OPTIMIZER_STATE(LogOptimizerState, internal::version_info)
};



/** \brief Dump the state of the model to a file on an error and then
    go on the the other handlers.

    When an error (check or assertion failure) occurs, the model is
    dumped to the specified using the geometry given in the
    LogOptimizerState.

    \untested(set_failure_dump)
 */
class IMPDISPLAYEXPORT DisplayModelOnFailure: public FailureHandler {
  Pointer<LogOptimizerState> s_;
  std::string file_name_;
 public:
  DisplayModelOnFailure(LogOptimizerState *m, std::string file_name);
  IMP_FAILURE_HANDLER(DisplayModelOnFailure, internal::version_info);
};

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_LOG_OPTIMIZER_STATE_H */
