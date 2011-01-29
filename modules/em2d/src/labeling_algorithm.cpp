/**
 *  \file labeling_algorithm.cpp
 *  \brief labeling images for EM
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/labeling_algorithm.h"
#include "IMP/algebra/utility.h"
#include <IMP/exception.h>


IMPEM2D_BEGIN_NAMESPACE

//
//void set_final_labels(algebra::Matrix2D<int> &mat_to_label,Ints &Labels) {
//  for (int i=mat_to_label.get_start(0);i<=mat_to_label.get_finish(0);++i) {
//    for (int j=mat_to_label.get_start(1);j<=mat_to_label.get_finish(1);++j) {
//      mat_to_label(i,j)=Labels[mat_to_label(i,j)];
//    }
//  }
//}


//
//
//void do_copy_tree(const Pixel &p, const Pixel &a,const Pixel &c,
//            algebra::Matrix2D<int> &mat_to_label,
//            Ints &Labels) {
//  mat_to_label(p) = do_union(Labels,mat_to_label(c),mat_to_label(a));
//}
//
//void do_copy_tree(const Pixel &p, const Pixel &a,
//            algebra::Matrix2D<int> &mat_to_label) {
//  mat_to_label(p) = mat_to_label(a);
//}


IMPEM2D_END_NAMESPACE
