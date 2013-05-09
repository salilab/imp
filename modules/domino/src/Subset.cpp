/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/Subset.h>

IMPDOMINO_BEGIN_NAMESPACE

std::string Subset::get_name() const {
  std::ostringstream oss;
  show(oss);
  return oss.str();
}

IMPDOMINO_END_NAMESPACE
