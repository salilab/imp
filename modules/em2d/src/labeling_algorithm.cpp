/**
 *  \file labeling_algorithm.cpp
 *  \brief labeling images for EM
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/labeling_algorithm.h"
#include "IMP/algebra/utility.h"
#include <IMP/exception.h>


IMPEM2D_BEGIN_NAMESPACE


void assign_final_labels(algebra::Matrix2D<int> &L,std::vector<int> &P) {
  for (int i=L.get_start(0);i<=L.get_finish(0);++i) {
    for (int j=L.get_start(1);j<=L.get_finish(1);++j) {
      L(i,j)=P[L(i,j)];
    }
  }
}


int p_union(std::vector<int> &P,const int i,const int j) {
  int root = find_root(P,i);
  if(i != j) {
    int root_j = find_root(P,j);
    // Check which is the lowest root label
    if(root > root_j) {
      root = root_j;
    }
    // Set the lowest label
    set_root(P,j,root);
  }
  set_root(P,i,root);
  return root;
}



void d_copy(const Pixel &p, const Pixel &a,const Pixel &c,
            algebra::Matrix2D<int> &L,
            std::vector<int> &P) {
  L(p) = p_union(P,L(c),L(a));
}

void d_copy(const Pixel &p, const Pixel &a,
            algebra::Matrix2D<int> &L) {
  L(p) = L(a);
}


IMPEM2D_END_NAMESPACE
