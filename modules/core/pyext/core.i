%module(directors="1") "IMP.core"

%{
#include "IMP.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "core_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"


namespace IMP {
  namespace core {
    %include "IMP/core/macros.h"
  }
}


/* Add additional IMPCORE_CONTAINER methods for scripting languages
   when the plural container is in the core namespace. */
%define IMPCORE_CONTAINER_SWIG(type, Ucname, lcname)
%extend type {
  IMP::core::Ucname##s get_##lcname##s() const {
    IMP::core::Ucname##s ret(self->lcname##s_begin(), self->lcname##s_end());
    return ret;
  }
}
%enddef


/* Get definitions of base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "kernel/pyext/IMP_keys.i"
%import "modules/algebra/pyext/algebra.i"
%include "singleton_containers.i"
%include "pair_containers.i"

namespace IMP {
  namespace core {
    IMP_CONTAINER_SWIG(RestraintSet, Restraint, restraint)
    IMPCORE_CONTAINER_SWIG(MonteCarlo, Mover, mover)

    //IMP_CONTAINER_SWIG(FilteredListSingletonContainer, Particle, singleton)
    //IMP_CONTAINER_SWIG(ListSingletonContainer, Particle, singleton)
  }
}



/* Don't wrap internal classes */
%ignore IMP::core::internal::ChildArrayTraits;

/* Wrap our own base classes */
%include "IMP/core/ClosePairsFinder.h"
%include "IMP/core/Mover.h"
%include "IMP/core/MoverBase.h"

%include "IMP/core/XYZDecorator.h"

/* Must be included before HierarchyDecorator.h since it is not easy
   to predeclare a typedef (for BondDecorators) */ 
%include "IMP/core/RestraintSet.h"

/* Wrap the final classes */
%include "IMP/core/AllPairsPairContainer.h"
%include "IMP/core/AngleRestraint.h"
%include "IMP/core/AngleTripletScore.h"
%include "IMP/core/AttributeSingletonScore.h"
%include "IMP/core/BallMover.h"
%include "IMP/core/BoxSweepClosePairsFinder.h"
%include "IMP/core/CentroidOfRefined.h"
%include "IMP/core/ChildrenRefiner.h"
%include "IMP/core/ClosedCubicSpline.h"
%include "IMP/core/ClosePairsScoreState.h"
%include "IMP/core/CloseBipartitePairsScoreState.h"
%include "IMP/core/ClosePairsPairScore.h"
%include "IMP/core/ConjugateGradients.h"
%include "IMP/core/ConnectivityRestraint.h"
%include "IMP/core/ConstantRestraint.h"
%include "IMP/core/Cosine.h"
%include "IMP/core/CoverRefined.h"
%include "IMP/core/DerivativesFromRefined.h"
%include "IMP/core/DerivativesToRefined.h"
%include "IMP/core/DiameterRestraint.h"
%include "IMP/core/Transform.h"
%include "IMP/core/DihedralRestraint.h"
%include "IMP/core/DistancePairScore.h"
%include "IMP/core/DistanceRestraint.h"
%include "IMP/core/DistanceToSingletonScore.h"
%include "IMP/core/FixedRefiner.h"
%include "IMP/core/GridClosePairsFinder.h"
%include "IMP/core/Harmonic.h"
%include "IMP/core/HarmonicLowerBound.h"
%include "IMP/core/HarmonicUpperBound.h"
%include "IMP/core/HierarchyDecorator.h"
%include "IMP/core/LeavesRefiner.h"
%include "IMP/core/Linear.h"
%include "IMP/core/MaximumChangeScoreState.h"
%include "IMP/core/MonteCarlo.h"
%include "IMP/core/NameDecorator.h"
%include "IMP/core/NormalMover.h"
%include "IMP/core/OpenCubicSpline.h"
%include "IMP/core/QuadraticClosePairsFinder.h"
%include "IMP/core/RefinedPairsPairScore.h"
%include "IMP/core/rigid_bodies.h"
%include "IMP/core/SphereDistancePairScore.h"
%include "IMP/core/SteepestDescent.h"
%include "IMP/core/TransformedDistancePairScore.h"
%include "IMP/core/TypedPairScore.h"
%include "IMP/core/XYZRDecorator.h"
%include "IMP/core/model_io.h"
%include "IMP/core/TableRefiner.h"
%include "IMP/core/TruncatedHarmonic.h"

%include "IMP/core/ListPairContainer.h"
%include "IMP/core/PairsRestraint.h"
%include "IMP/core/PairRestraint.h"
%include "IMP/core/FilteredListSingletonContainer.h"
%include "IMP/core/FilteredListPairContainer.h"
%include "IMP/core/PairsScoreState.h"
%include "IMP/core/PairScoreState.h"
%include "IMP/core/SingletonsRestraint.h"
%include "IMP/core/SingletonRestraint.h"
%include "IMP/core/ListSingletonContainer.h"
%include "IMP/core/PairContainerSet.h"
%include "IMP/core/SingletonContainerSet.h"
%include "IMP/core/SingletonsScoreState.h"
%include "IMP/core/SingletonScoreState.h"
%include "IMP/core/MinimumSingletonScoreRestraint.h"
%include "IMP/core/MinimumPairScoreRestraint.h"
%include "IMP/core/MaximumSingletonScoreRestraint.h"
%include "IMP/core/MaximumPairScoreRestraint.h"
%include "IMP/core/RigidBodyMover.h"
%include "IMP/core/utility.h"



namespace IMP {
  namespace core {
    %template(Movers) ::std::vector<Mover*>;
    %template(XYZDecorators) ::std::vector<XYZDecorator>;
    %template(XYZRDecorators) ::std::vector<XYZRDecorator>;
    %template(RigidMemberDecorators) ::std::vector<RigidMemberDecorator>;
    %template(TruncatedHarmonicLowerBound) ::IMP::core::TruncatedHarmonic<LOWER>;
    %template(TruncatedHarmonicUpperBound) ::IMP::core::TruncatedHarmonic<UPPER>;
    %template(TruncatedHarmonicBound) ::IMP::core::TruncatedHarmonic<BOTH>;
    // swig screws up on scopes, I can't be bothered to fix it
    //%template(show_named_hierarchy) show<::IMP::core::NameDecorator>;
  }
}
