/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/internal/utility.h"
#include "IMP/ScoreState.h"
#include "IMP/ModelObject.h"
IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

void show_dg_node(ModelObject *vertex, TextOutput out) {
  out << vertex->get_name() << "\\n[" << vertex->get_type_name();
  ScoreState *ss = dynamic_cast<ScoreState *>(vertex);
  if (ss && ss->get_has_update_order()) {
    out << ": " << ss->get_update_order();
  }
  out << "]";
}

IMPKERNEL_END_INTERNAL_NAMESPACE
