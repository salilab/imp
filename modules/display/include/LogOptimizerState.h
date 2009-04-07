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
  Pointer<Writer> writer_;
  unsigned int step_;
  unsigned int skip_steps_;
  std::string name_template_;

  std::vector<Pointer<CompoundGeometry> > edata_;
  std::vector<Pointer<Geometry> > gdata_;

public:
  //! Write files using name_template as a template (must have a %d in it)
  LogOptimizerState(Writer *writer, std::string name_template);

  virtual ~LogOptimizerState();

  //! Set the number of steps to skip
  void set_skip_steps(unsigned int i) {
    skip_steps_=i;
  }

  void add_geometry(CompoundGeometry* g) {
    edata_.push_back(Pointer<CompoundGeometry>(g));
  }

  void add_geometry(Geometry* g) {
    gdata_.push_back(Pointer<Geometry>(g));
  }

  void add_geometry(const CompoundGeometries& g) {
    for (unsigned int i=0; i< g.size(); ++i) {
      add_geometry(g);
    }
  }

  void add_geometry(const Geometries& g) {
    for (unsigned int i=0; i< g.size(); ++i) {
      add_geometry(g);
    }
  }

  IMP_OPTIMIZER_STATE(internal::version_info)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_LOG_OPTIMIZER_STATE_H */
