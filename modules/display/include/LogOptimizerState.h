/**
 *  \file IMP/display/LogOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_LOG_OPTIMIZER_STATE_H
#define IMPDISPLAY_LOG_OPTIMIZER_STATE_H

#include <IMP/display/display_config.h>
#include "declare_Geometry.h"
#include "Writer.h"
#include <IMP/kernel/OptimizerState.h>
#include <IMP/display/geometry.h>
#include <IMP/base/Pointer.h>
#include <IMP/kernel/internal/utility.h>
#include <vector>

IMPDISPLAY_BEGIN_NAMESPACE

/** Write to a Writer periodically.
 */
class IMPDISPLAYEXPORT WriteOptimizerState : public kernel::OptimizerState {
  IMP::base::PointerMember<Writer> writer_;
 public:
  /** \deprecated_at{2.1} Use the method that takes a kernel::Model. */
  IMPDISPLAY_DEPRECATED_FUNCTION_DECL(2.1)
  WriteOptimizerState(WriterAdaptor w);
  WriteOptimizerState(kernel::Model *m, WriterAdaptor w);
  void write(WriterAdaptor w) const;
  IMP_LIST_ACTION(public, Geometry, Geometries, geometry, geometries, Geometry*,
                  Geometries, , , );
 protected:
  virtual void do_update(unsigned int) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(WriteOptimizerState);
};

IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_LOG_OPTIMIZER_STATE_H */
