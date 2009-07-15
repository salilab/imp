/**
 *  \file distance.cpp  \brief distance measures
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/distance.h>
#include <IMP/algebra/VectorD.h>
IMPATOM_BEGIN_NAMESPACE

double rmsd(const core::XYZs& m1 ,const core::XYZs& m2) {
  IMP_check(m1.size()==m2.size(),
            "The input sets of XYZ points "
            <<"should be of the same size", ValueException);
  float rmsd=0.0;
  for(unsigned int i=0;i<m1.size();i++) {
    rmsd += algebra::squared_distance(m1[i].get_coordinates()
                                     ,m2[i].get_coordinates());
  }
  return std::sqrt(rmsd / m1.size());
}

IMPATOM_END_NAMESPACE
