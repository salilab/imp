/**
 *  \file LogOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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
    printf-style format string.
    \verbinclude log.py
    \ingroup log
 */
class IMPDISPLAYEXPORT LogOptimizerState: public OptimizerState
{
  Pointer<Writer> writer_;
  unsigned int step_;
  unsigned int skip_steps_;
  std::string name_template_;
  typedef std::pair<Pointer<GeometryExtractor>,
    Pointer<SingletonContainer> > EPair;
  std::vector< EPair > edata_;
  std::vector<Pointer<Geometry> > gdata_;

  EPair make_pair(GeometryExtractor* g, SingletonContainer *p) const {
    return std::make_pair(Pointer<GeometryExtractor>(g),
                          Pointer<SingletonContainer>(p));
  }
public:
  //! Write files using name_template as a template (must have a %d in it)
  LogOptimizerState(Writer *writer, std::string name_template);

  virtual ~LogOptimizerState();

  //! Set the number of steps to skip
  void set_skip_steps(unsigned int i) {
    skip_steps_=i;
  }

  //! Add to the list of what to display
  void add_geometry_extractor(GeometryExtractor* g, SingletonContainer *p) {
    edata_.push_back(make_pair(g, p));
  }

  //! Add to the list of what to display
  void add_geometry(Geometry* g) {
    gdata_.push_back(Pointer<Geometry>(g));
  }
  IMP_OPTIMIZER_STATE(internal::version_info)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_LOG_OPTIMIZER_STATE_H */
