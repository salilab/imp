/**
 *  \file TAMDCentroid.cpp   \brief Simple xyzr decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/TAMDCentroid.h"
#include <IMP/constants.h>

IMPATOM_BEGIN_NAMESPACE

IntKey TAMDCentroid::get_is_tamd_centroid_key() {
  static IntKey k("TAMDCentroid__is_tamd_centroid");
  return k;
}

void TAMDCentroid::show(std::ostream &out) const {
  out << "TAMD Centroid";
}

IMPATOM_END_NAMESPACE
