/**
 *  \file Writer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/LogOptimizerState.h"
#include <IMP/base/Pointer.h>

IMPDISPLAY_BEGIN_NAMESPACE

void WriteOptimizerState::write(WriterAdaptor w) const {
  IMP::base::PointerMember<Writer> wp(w);
  for (unsigned int i = 0; i < get_number_of_geometries(); ++i) {
    get_geometry(i)->set_was_used(true);
    w->add_geometry(get_geometry(i));
  }
}

void WriteOptimizerState::update() {
  if (call_number_ % (skip_steps_ + 1) == 0) {
    writer_->set_frame(update_number_);
    write(writer_.get());
    ++update_number_;
  }
  ++call_number_;
}
IMP_LIST_ACTION_IMPL(WriteOptimizerState, Geometry, Geometries, geometry,
                     geometries, Geometry*, Geometries);

IMPDISPLAY_END_NAMESPACE
