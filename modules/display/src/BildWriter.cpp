/**
 *  \file BildWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/BildWriter.h"
#include <IMP/base/deprecation_macros.h>

IMPDISPLAY_BEGIN_NAMESPACE
#if IMP_USE_DEPRECATED

void BildWriter::do_open() {
  IMP_DEPRECATED_OBJECT(IMP::rmf);
}

void BildWriter::do_close() {
}
#endif
IMPDISPLAY_END_NAMESPACE
