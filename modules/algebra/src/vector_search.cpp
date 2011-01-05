/**
 *  \file vector_search.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/vector_search.h>

IMPALGEBRA_BEGIN_NAMESPACE

#ifdef IMP_ALGEBRA_USE_ANN
// make sure symbols are included
namespace {
  class ANNInclude: public internal::ANNData<3>{
    ANNInclude(std::vector<VectorD<3> > vs):
      internal::ANNData<3>(vs.begin(), vs.end()){
      Ints ret;
      internal::ANNData<3>::fill_nearest_neighbors(vs[0], 1U, 0, ret);
      internal::ANNData<3>::get_point(0);
      internal::ANNData<3>::get_number_of_points();
      internal::ANNData<3>::fill_nearest_neighbors(vs[0], 0.0, 0.0, ret);
    }
  };
}
#endif
IMPALGEBRA_END_NAMESPACE
