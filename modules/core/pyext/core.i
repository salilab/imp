%module(directors="1") IMP_core

%{
#include "IMP.h"
#include "IMP/core.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "core_exports.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

namespace IMP {
  namespace core {
    %include "IMP/core/macros.h"
  }
}

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "kernel/pyext/IMP_keys.i"

namespace IMP {
  namespace core {
    %template(AtomTypeBase) ::IMP::KeyBase<IMP_ATOM_TYPE_INDEX>;
    %template(ResidueTypeBase) ::IMP::KeyBase<IMP_RESIDUE_TYPE_INDEX>;

    IMP_OWN_FIRST_CONSTRUCTOR(DistancePairScore)
    IMP_OWN_FIRST_CONSTRUCTOR(TransformedDistancePairScore)
    IMP_OWN_FIRST_CONSTRUCTOR(SphereDistancePairScore)
    IMP_SET_OBJECT(TypedPairScore, set_pair_score)

    IMP_OWN_FIRST_CONSTRUCTOR(DistanceToSingletonScore)
    IMP_OWN_FIRST_CONSTRUCTOR(AttributeSingletonScore)
    IMP_OWN_FIRST_CONSTRUCTOR(AngleTripletScore)
    IMP_OWN_FIRST_CONSTRUCTOR(AngleRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(DistanceRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(DihedralRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(ConnectivityRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(NonbondedRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(BondDecoratorRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(SingletonListRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(PairListRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(TripletChainRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(PairChainRestraint)
    IMP_CONTAINER_SWIG(RestraintSet, Restraint, restraint)

    IMP_SET_OBJECT(MonteCarlo, set_local_optimizer)

    IMP_ADD_OBJECT(MonteCarlo, add_mover)
    IMP_ADD_OBJECTS(MonteCarlo, add_movers)
    IMP_ADD_OBJECT(NonbondedListScoreState, add_bonded_list)
    IMP_ADD_OBJECTS(NonbondedListScoreState, add_bonded_lists)
  }
}

/* Don't wrap internal classes */
%ignore IMP::core::internal::ChildArrayTraits;

/* Wrap our own classes */
%include "IMP/core/bond_decorators.h"
%include "IMP/core/HierarchyDecorator.h"
%include "IMP/core/MolecularHierarchyDecorator.h"
%include "IMP/core/NameDecorator.h"
%include "IMP/core/ResidueDecorator.h"
%include "IMP/core/XYZDecorator.h"
%include "IMP/core/AtomDecorator.h"
%include "IMP/core/Cosine.h"
%include "IMP/core/Harmonic.h"
%include "IMP/core/HarmonicLowerBound.h"
%include "IMP/core/HarmonicUpperBound.h"
%include "IMP/core/OpenCubicSpline.h"
%include "IMP/core/ClosedCubicSpline.h"
%include "IMP/core/Linear.h"
%include "IMP/core/DistanceToSingletonScore.h"
%include "IMP/core/AttributeSingletonScore.h"
%include "IMP/core/AngleTripletScore.h"
%include "IMP/core/BondedListScoreState.h"
%include "IMP/core/MaxChangeScoreState.h"
%include "IMP/core/NonbondedListScoreState.h"
%include "IMP/core/AllNonbondedListScoreState.h"
%include "IMP/core/BondDecoratorListScoreState.h"
%include "IMP/core/BipartiteNonbondedListScoreState.h"
%include "IMP/core/GravityCenterScoreState.h"
%include "IMP/core/DistanceRestraint.h"
%include "IMP/core/RestraintSet.h"
%include "IMP/core/ConnectivityRestraint.h"
%include "IMP/core/BondDecoratorRestraint.h"
%include "IMP/core/NonbondedRestraint.h"
%include "IMP/core/PairChainRestraint.h"
%include "IMP/core/PairListRestraint.h"
%include "IMP/core/SingletonListRestraint.h"
%include "IMP/core/TripletChainRestraint.h"
%include "IMP/core/AngleRestraint.h"
%include "IMP/core/ConstantRestraint.h"
%include "IMP/core/DihedralRestraint.h"
%include "IMP/core/SteepestDescent.h"
%include "IMP/core/ConjugateGradients.h"
%include "IMP/core/MolecularDynamics.h"
%include "IMP/core/BrownianDynamics.h"
%include "IMP/core/Mover.h"
%include "IMP/core/MoverBase.h"
%include "IMP/core/MonteCarlo.h"
%include "IMP/core/BallMover.h"
%include "IMP/core/NormalMover.h"
%include "IMP/core/VRMLLogOptimizerState.h"
%include "IMP/core/CMMLogOptimizerState.h"
%include "IMP/core/VelocityScalingOptimizerState.h"
%include "IMP/core/DistancePairScore.h"
%include "IMP/core/SphereDistancePairScore.h"
%include "IMP/core/TypedPairScore.h"
%include "IMP/core/TransformedDistancePairScore.h"

namespace IMP {
  namespace core {
    %template(show_named_hierarchy) show<NameDecorator>;
    %template(show_molecular_hierarchy) show<MolecularHierarchyDecorator>;
    %template(MoverIndex) ::IMP::Index<Mover>;
    %template(BondedListIndex) ::IMP::Index<BondedListScoreState>;
    %template(BondDecorators) ::std::vector<BondDecorator>;
  }
}
