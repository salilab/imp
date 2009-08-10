%module(directors="1") "IMP.misc"

%{
#include "IMP.h"
#include "IMP/misc.h"
%}


%include "IMP_macros.i"
%include "IMP_exceptions.i"
%include "IMP_streams.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of base classes (but do not wrap) */
%import "kernel.i"
%import "algebra.i"

%include "IMP/misc/config.h"

namespace IMP {
  typedef VectorD<3> Vector3D;
}

/* Wrap our own classes */
%include "IMP/misc/WormLikeChain.h"
%include "IMP/misc/LowestRefinedPairScore.h"
%include "IMP/misc/RefineOncePairScore.h"
%include "IMP/misc/attribute_statistics.h"
%include "IMP/misc/LogPairScore.h"
%include "IMP/misc/StateAdaptor.h"
%include "IMP/misc/CommonEndpointPairContainer.h"
