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
double rmsd(const Hierarchy& m1 ,const Hierarchy& m2) {
  IMP::Particles ps1,ps2;
  ps1 = IMP::core::get_leaves(m1);
  ps2 = IMP::core::get_leaves(m2);
  IMP_check(ps1.size()==ps2.size(),
            "proteins given as input to rmsd calculation "
            <<"should be of the same size", ValueException);
  float rmsd=0.0;
  for(unsigned int i=0;i<ps1.size();i++) {
    rmsd += IMP::algebra::squared_distance(core::XYZ(ps1[i]).get_coordinates(),
                                        core::XYZ(ps2[i]).get_coordinates());
  }
  return std::sqrt(rmsd / ps1.size());
}

IMPATOM_END_NAMESPACE
