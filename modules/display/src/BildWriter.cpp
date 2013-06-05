/**
 *  \file BildWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/BildWriter.h"
#include <IMP/base/deprecation_macros.h>

IMPDISPLAY_BEGIN_NAMESPACE


void BildWriter::do_open() {
  IMPDISPLAY_DEPRECATED_CLASS_DEF(2.1, "Use IMP::rmf");
}

void BildWriter::do_close() {}

IMPDISPLAY_END_NAMESPACE
