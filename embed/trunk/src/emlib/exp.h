#ifndef EXP_APPROXIMATION
#define EXP_APPROXIMATION

/* An approximation of the exponential function. 
   Schraudolph, Nicol N. "A Fast, Compact Approximation of the exponential
   function" Technical Report IDSIA-07-98
   (Neural Computation 11(4))
   The function returns values within 1-2% deviation of the correct ones in
   the range (-700,700)
*/

#include<math.h>

typedef union
{
  double d;
  struct
  {
#ifdef EM_LITTLE_ENDIAN
    int j,i;
#else
    int i,j;
#endif
  } n;
} _eco;

#define EXP_A (1048576/M_LN2)
#define EXP_C 60801




inline double EXP(float y) {
  static _eco eco; 
  eco.n.i = (int) (EXP_A*(y) + (1072693248-EXP_C)); 
  return eco.d;
}

#endif
