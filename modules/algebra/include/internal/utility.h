/**
 *  \file algebra/internal/utility.h
 *  \brief Functions to deal with very common math operations
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_UTILITY_H
#define IMPALGEBRA_INTERNAL_UTILITY_H

#include "../config.h"
#include "tnt_array2d.h"
#include "../VectorD.h"
#include <vector>
IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

// defined in eigen analysis
IMPALGEBRAEXPORT TNT::Array2D<double>
   get_covariance_matrix( const std::vector<VectorD<3> > &vs,
                          const VectorD<3>& mean);



IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_UTILITY_H */
