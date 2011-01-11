/**
 *  \file test_grid.cpp   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/algebra/Grid3D.h>
#include <IMP/algebra/vector_generators.h>
#include <algorithm>

using namespace IMP::algebra;

int main(int, char *[]) {
  {
    typedef grids::UnboundedGridStorageD<4> UBGS;
    UBGS ubgs;
    int lb[]={1,2,3,4};
    int ub[]={2,4,6,8};
    grids::ExtendedGridIndexD<4> elb(lb, lb+4);
    grids::ExtendedGridIndexD<4> eub(ub, ub+4);
    std::cout << "eus " << elb << " " << eub << std::endl;
    std::vector<grids::ExtendedGridIndexD<4> >ids
      = ubgs.get_extended_indexes(elb, eub);
    for (unsigned int i=0; i< ids.size(); ++i) {
      std::cout << ids[i] << "\n";
    }
    std::cout << ids.size() << std::endl;
    IMP_INTERNAL_CHECK(ids.size()==2*3*4*5, "Sizes don't match");
  }
  return EXIT_SUCCESS;
}
