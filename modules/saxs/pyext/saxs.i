%module(directors="1") "IMP.saxs"

%{
#include "IMP.h"
#include "IMP/saxs.h"
%}


%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "saxs_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
/*
namespace IMP {
  typedef VectorD<3> Vector3D;
}

namespace IMP::saxs {
  IMP_OWN_FIRST_CONSTRUCTOR(BondCoverPairScore)
  IMP_OWN_FIRST_SECOND_CONSTRUCTOR(RefineOncePairScore)
  IMP_OWN_FIRST_SECOND_CONSTRUCTOR(LowestRefinedPairScore)
  IMP_OWN_FIRST_CONSTRUCTOR(TunnelSingletonScore)
}
*/
