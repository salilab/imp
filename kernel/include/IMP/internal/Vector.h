/**
 *  \file Vector.h    \brief A bounds checked vector.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_VECTOR_H
#define __IMP_VECTOR_H

#include "../exception.h"

#include <vector>

namespace IMP
{

namespace internal
{

//! A vector with bounds checking
/** This class is designed to be used from the IMP_LIST macro and 
    is not really designed to be used independently. 
    \internal
 */
template <class D>
class Vector: public std::vector<D>
{
  typedef std::vector<D> P;
public:
  Vector(){}
  const D& operator[](unsigned int i) const {
    IMP_check(i < P::size(),
              "Index " << i << " out of range",
              IndexException(""));
    return P::operator[](i);
  }
  D& operator[](unsigned int i) {
    IMP_check(i < P::size(),
              "Index " << i << " out of range",
              IndexException(""));
    return P::operator[](i);
  }
};


} // namespace internal

} // namespace IMP

#endif  /* __IMP_VECTOR_H */
