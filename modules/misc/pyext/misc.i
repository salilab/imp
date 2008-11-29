%module(directors="1") IMP_misc

%{
#include "IMP/misc.h"
#include "IMP/core.h"
#include "IMP.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "misc_exports.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "modules/core/pyext/core.i"

namespace IMP {
  typedef VectorD<3> Vector3D;
}

namespace IMP::misc {
  IMP_OWN_FIRST_CONSTRUCTOR(BondCoverPairScore)
  IMP_OWN_FIRST_SECOND_CONSTRUCTOR(RefineOncePairScore)
  IMP_OWN_FIRST_SECOND_CONSTRUCTOR(LowestRefinedPairScore)
  IMP_OWN_FIRST_CONSTRUCTOR(TunnelSingletonScore)
}


/* Wrap our own classes */
%include "IMP/misc/WormLikeChain.h"
%include "IMP/misc/ChildrenParticleRefiner.h"
%include "IMP/misc/RefineOncePairScore.h"
%include "IMP/misc/TunnelSingletonScore.h"
%include "IMP/misc/LowestRefinedPairScore.h"
%include "IMP/misc/Matrix3D.h"
%include "IMP/misc/Rotation3D.h"
%include "IMP/misc/Transformation3D.h"
%include "IMP/misc/ParticleFunction.h"
%include "IMP/misc/TransformationFunction.h"
