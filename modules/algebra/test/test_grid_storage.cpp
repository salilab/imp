/**
 *  \file test_grid.cpp   \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/algebra/standard_grids.h>
#include <IMP/algebra/vector_generators.h>
#include <algorithm>

using namespace IMP::algebra;

int main(int, char * []) {
  {
    typedef UnboundedGridRangeD<4> UBGS;
    UBGS ubgs;
    int lb[] = {1, 2, 3, 4};
    int ub[] = {2, 4, 6, 8};
    ExtendedGridIndexD<4> elb(lb, lb + 4);
    ExtendedGridIndexD<4> eub(ub, ub + 4);
    std::cout << "eus " << elb << " " << eub << std::endl;
    IMP::base::Vector<ExtendedGridIndexD<4> > ids =
        ubgs.get_extended_indexes(elb, eub);
    for (unsigned int i = 0; i < ids.size(); ++i) {
      std::cout << ids[i] << "\n";
    }
    std::cout << ids.size() << std::endl;
    IMP_INTERNAL_CHECK(ids.size() == 2 * 3 * 4 * 5, "Sizes don't match");
  }
  return EXIT_SUCCESS;
}
