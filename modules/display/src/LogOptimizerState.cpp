/**
 *  \file Writer.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/LogOptimizerState.h"

IMPDISPLAY_BEGIN_NAMESPACE

void WriteOptimizerState::write(WriterOutput w) const {
  IMP::internal::OwnerPointer<Writer> wp(w);
  for (unsigned int i=0; i< get_number_of_geometries(); ++i) {
    get_geometry(i)->set_was_used(true);
    w->add_geometry(get_geometry(i));
  }
}

void WriteOptimizerState::update() {
  if (call_number_%(skip_steps_+1) ==0) {
    writer_->set_frame(update_number_);
    write(writer_.get());
    ++update_number_;
  }
  ++call_number_;
}
IMP_LIST_PLURAL_IMPL(WriteOptimizerState, Geometry, Geometries,
                     geometry, geometries, Geometry*, Geometries,,,);

IMPDISPLAY_END_NAMESPACE
