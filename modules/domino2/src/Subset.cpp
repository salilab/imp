/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino2/Subset.h>



IMPDOMINO2_BEGIN_NAMESPACE

void Subset::show(std::ostream &out) const {
  out << "[";
  for (unsigned int i=0; i< size(); ++i) {
    out << ps_[i]->get_name() << " ";
  }
  out << "]";
}

std::string Subset::get_name() const {
  std::ostringstream oss;
  show(oss);
  return oss.str();
}

IMPDOMINO2_END_NAMESPACE
