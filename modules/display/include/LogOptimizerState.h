/**
 *  \file IMP/display/LogOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_LOG_OPTIMIZER_STATE_H
#define IMPDISPLAY_LOG_OPTIMIZER_STATE_H

#include <IMP/display/display_config.h>
#include "declare_Geometry.h"
#include "Writer.h"
#include <IMP/OptimizerState.h>
#include <IMP/display/geometry.h>
#include <IMP/Pointer.h>
#include <IMP/internal/utility.h>
#include <vector>

IMPDISPLAY_BEGIN_NAMESPACE

/** Write to a Writer periodically.
 */
class IMPDISPLAYEXPORT WriteOptimizerState : public OptimizerState {
  IMP::PointerMember<Writer> writer_;

 public:
  WriteOptimizerState(Model *m, WriterAdaptor w);
  void write(WriterAdaptor w) const;
  IMP_LIST_ACTION(public, Geometry, Geometries, geometry, geometries,
                  Geometry *, Geometries, , , );

 protected:
  virtual void do_update(unsigned int) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(WriteOptimizerState);
};

IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_LOG_OPTIMIZER_STATE_H */
