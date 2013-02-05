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
#include <IMP/OptimizerState.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/geometry.h>
#include <IMP/Pointer.h>
#include <IMP/internal/utility.h>
#include <vector>

IMPDISPLAY_BEGIN_NAMESPACE

/** Write to a Writer periodically.
 */
class IMPDISPLAYEXPORT WriteOptimizerState: public OptimizerState {
  ::IMP::kernel::internal::Counter skip_steps_, call_number_, update_number_;
  IMP::OwnerPointer<Writer> writer_;
  void update();
 public:
  WriteOptimizerState(WriterAdaptor w) :
    OptimizerState("WriteOptimizerState%1%"),
    writer_(w) {}
  void set_period(unsigned int p) {
    skip_steps_=p-1;
    call_number_=0;
  }
  IMP_LIST_ACTION(public, Geometry, Geometries, geometry, geometries,
                  Geometry*, Geometries,,,);
  void write(WriterAdaptor w) const;
  IMP_OBJECT_INLINE(WriteOptimizerState,
                    out << "  writer: " << writer_->get_name() << std::endl;,);
};
IMP_OBJECTS(WriteOptimizerState, WriteOptimizerStates);


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_LOG_OPTIMIZER_STATE_H */
