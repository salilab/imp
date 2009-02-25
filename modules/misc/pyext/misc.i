%module(directors="1") "IMP.misc"

%{
#include "IMP.h"
#include "IMP/misc.h"
%}


%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "misc_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "modules/algebra/pyext/algebra.i"

namespace IMP {
  typedef VectorD<3> Vector3D;
}

namespace IMP::misc {
  IMP_OWN_CONSTRUCTOR(BondCoverPairScore)
  IMP_OWN_CONSTRUCTOR(RefineOncePairScore)
  IMP_OWN_CONSTRUCTOR(LowestRefinedPairScore)
  IMP_OWN_CONSTRUCTOR(TunnelSingletonScore)
  IMP_OWN_CONSTRUCTOR(StateAdaptor)
}


/* Wrap our own classes */
%include "IMP/misc/WormLikeChain.h"
%include "IMP/misc/TunnelSingletonScore.h"
%include "IMP/misc/LowestRefinedPairScore.h"
%include "IMP/misc/RefineOncePairScore.h"
%include "IMP/misc/attribute_statistics.h"
%include "IMP/misc/LogPairScore.h"
%include "IMP/misc/StateAdaptor.h"
