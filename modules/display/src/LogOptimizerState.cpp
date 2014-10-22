/**
 *  \file Writer.cpp
 *  \brief Base class for writing geometry to a file.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/LogOptimizerState.h"
#include <IMP/base/Pointer.h>

IMPDISPLAY_BEGIN_NAMESPACE
WriteOptimizerState::WriteOptimizerState(kernel::Model *m, WriterAdaptor w)
    : kernel::OptimizerState(m, "WriteOptimizerState%1%"), writer_(w) {}

void WriteOptimizerState::write(WriterAdaptor w) const {
  IMP::base::PointerMember<Writer> wp(w);
  for (unsigned int i = 0; i < get_number_of_geometries(); ++i) {
    get_geometry(i)->set_was_used(true);
    w->add_geometry(get_geometry(i));
  }
}

void WriteOptimizerState::do_update(unsigned int frame) {
  writer_->set_frame(frame);
  write(writer_.get());
}

IMP_LIST_ACTION_IMPL(WriteOptimizerState, Geometry, Geometries, geometry,
                     geometries, Geometry *, Geometries);

IMPDISPLAY_END_NAMESPACE
