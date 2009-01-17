/*
 * \file tnt_math_utils.h
 * \brief tnt_math_utils.h
 */

#ifndef IMPALGEBRA_TNT_MATH_UTILS_H
#define IMPALGEBRA_TNT_MATH_UTILS_H

/* needed for fabs, sqrt() below */
#include "config.h"
#include <cmath>



IMPALGEBRA_BEGIN_NAMESPACE
namespace TNT
{
using namespace std;
/**
  @returns hypotenuse of real (non-complex) scalars a and b by
  avoiding underflow/overflow
  using (a * sqrt( 1 + (b/a) * (b/a))), rather than
  sqrt(a*a + b*b).
*/
template <class Real>
Real hypot(const Real &a, const Real &b)
{

  if (a== 0)
    return abs(b);
  else
  {
    Real c = b/a;
    return fabs(a) * sqrt(1 + c*c);
  }
}
} /* TNT namespace */

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_TNT_MATH_UTILS_H */
