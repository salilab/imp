/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/Subset.h>



IMPDOMINO_BEGIN_NAMESPACE

void Subset::show(std::ostream &out) const {
  out << "[";
  for (unsigned int i=0; i< size(); ++i) {
    out << "\"" <<  ps_[i]->get_name() << "\" ";
  }
  out << "]";
}

std::string Subset::get_name() const {
  std::ostringstream oss;
  show(oss);
  return oss.str();
}

IMPDOMINO_END_NAMESPACE
