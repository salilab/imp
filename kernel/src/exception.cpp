/**
 *  \file exception.cpp   \brief Check handling.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/exception.h"

namespace IMP
{

static CheckLevel check_mode =
#ifdef NDEBUG
  CHEAP;
#else
  EXPENSIVE;
#endif

void set_check_level(CheckLevel cm)
{
  check_mode = cm;
}

CheckLevel get_check_level()
{
  return check_mode;
}

} // namespace IMP
