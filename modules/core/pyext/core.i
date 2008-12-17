%module(directors="1") "IMP.core"

%{
#include "IMP.h"
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
IMP_ADD_OBJECT(type, add_##lcname)
IMP_ADD_OBJECTS(type, add_##lcname##s)
%enddef


/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "kernel/pyext/IMP_keys.i"
%include "singleton_containers.i"
%include "pair_containers.i"

namespace IMP {
  typedef VectorD<3> Vector3D;
}

namespace IMP {
  namespace core {
    %template(AtomTypeBase) ::IMP::KeyBase<IMP_ATOM_TYPE_INDEX>;
    %template(ResidueTypeBase) ::IMP::KeyBase<IMP_RESIDUE_TYPE_INDEX>;

    IMP_OWN_FIRST_CONSTRUCTOR(AllPairsPairContainer)
    IMP_OWN_FIRST_CONSTRUCTOR(AngleRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(AngleTripletScore)
    IMP_OWN_FIRST_CONSTRUCTOR(AttributeSingletonScore)
    IMP_OWN_FIRST_CONSTRUCTOR(BondDecoratorRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(BondSingletonScore)
    IMP_OWN_FIRST_CONSTRUCTOR(ConnectivityRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(CoverRefinedSingletonModifier)
    IMP_OWN_FIRST_CONSTRUCTOR(CentroidOfRefinedSingletonModifier)
    IMP_OWN_FIRST_CONSTRUCTOR(DerivativesFromRefinedSingletonModifier)
    IMP_OWN_FIRST_CONSTRUCTOR(DerivativesToRefinedSingletonModifier)
    IMP_OWN_FIRST_CONSTRUCTOR(DihedralRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(DistancePairScore)
    IMP_OWN_FIRST_CONSTRUCTOR(DistanceRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(DistanceToSingletonScore)
    IMP_OWN_FIRST_CONSTRUCTOR(MaximumChangeScoreState)
    IMP_OWN_FIRST_CONSTRUCTOR(NonbondedRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(BondBondPairScore)
    IMP_OWN_FIRST_CONSTRUCTOR(PairChainRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(PairListRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(SingletonListRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(SphereDistancePairScore)
    IMP_OWN_FIRST_CONSTRUCTOR(TransformedDistancePairScore)
    IMP_OWN_FIRST_CONSTRUCTOR(TripletChainRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(VRMLLogOptimizerState)
    IMP_CONTAINER_SWIG(VRMLLogOptimizerState,
    ParticleRefiner, particle_refiner)
    IMP_OWN_FIRST_SECOND_CONSTRUCTOR(ClosePairsScoreState)
    IMP_OWN_FIRST_SECOND_THIRD_CONSTRUCTOR(CloseBipartitePairsScoreState)
    IMP_CONTAINER_SWIG(RestraintSet, Restraint, restraint)
    IMPCORE_CONTAINER_SWIG(MonteCarlo, Mover, mover)

    IMP_SET_OBJECT(MonteCarlo, set_local_optimizer)
    IMP_SET_OBJECT(TypedPairScore, set_pair_score)
    IMP_SET_OBJECT(ClosePairsScoreState, set_close_pairs_finder)
    IMP_SET_OBJECT(CloseBipartitePairsScoreState, set_close_pairs_finder)
    IMP_SET_OBJECT(VRMLLogOptimizerState, set_singleton_container)
    //IMP_CONTAINER_SWIG(FilteredListSingletonContainer, Particle, singleton)
    //IMP_CONTAINER_SWIG(ListSingletonContainer, Particle, singleton)
    
    IMP_ADD_OBJECT(NonbondedListScoreState, add_bonded_list)
    IMP_ADD_OBJECTS(NonbondedListScoreState, add_bonded_lists)
  }
}


%feature("director") IMP::core::SingletonContainer;
%feature("director") IMP::core::PairContainer;

// for kernel
%feature("director") IMP::core::SingletonModifier;
%feature("director") IMP::core::PairModifier;


/* Don't wrap internal classes */
%ignore IMP::core::internal::ChildArrayTraits;

/* Wrap our own base classes */
%include "IMP/core/ClosePairsFinder.h"
%include "IMP/core/Mover.h"
%include "IMP/core/MoverBase.h"
// should go to kernel
%include "IMP/core/SingletonContainer.h"
%include "IMP/core/PairContainer.h"
%include "IMP/core/SingletonModifier.h"
%include "IMP/core/PairModifier.h"

%include "IMP/core/XYZDecorator.h"
%include "IMP/core/BondedListScoreState.h"
%include "IMP/core/NonbondedListScoreState.h"

/* Must be included before HierarchyDecorator.h since it is not easy
   to predeclare a typedef (for BondDecorators) */ 
%include "IMP/core/bond_decorators.h"


/* Wrap the final classes */
%include "IMP/core/AllNonbondedListScoreState.h"
%include "IMP/core/AllPairsPairContainer.h"
%include "IMP/core/AngleRestraint.h"
%include "IMP/core/AngleTripletScore.h"
%include "IMP/core/AtomDecorator.h"
%include "IMP/core/AttributeSingletonScore.h"
%include "IMP/core/BallMover.h"
%include "IMP/core/BipartiteNonbondedListScoreState.h"
%include "IMP/core/BondDecoratorListScoreState.h"
%include "IMP/core/BondEndpointsParticleRefiner.h"
%include "IMP/core/BondPairContainer.h"
%include "IMP/core/BondDecoratorRestraint.h"
%include "IMP/core/BondSingletonScore.h"
%include "IMP/core/BondBondPairScore.h"
%include "IMP/core/BoxSweepClosePairsFinder.h"
%include "IMP/core/BrownianDynamics.h"
%include "IMP/core/CentroidOfRefinedSingletonModifier.h"
%include "IMP/core/ChildrenParticleRefiner.h"
%include "IMP/core/CMMLogOptimizerState.h"
%include "IMP/core/ClosedCubicSpline.h"
%include "IMP/core/ClosePairsScoreState.h"
%include "IMP/core/CloseBipartitePairsScoreState.h"
%include "IMP/core/ConjugateGradients.h"
%include "IMP/core/ConnectivityRestraint.h"
%include "IMP/core/ConstantRestraint.h"
%include "IMP/core/Cosine.h"
%include "IMP/core/CoverRefinedSingletonModifier.h"
%include "IMP/core/DerivativesFromRefinedSingletonModifier.h"
%include "IMP/core/DerivativesToRefinedSingletonModifier.h"
%include "IMP/core/DihedralRestraint.h"
%include "IMP/core/DistancePairScore.h"
%include "IMP/core/DistanceRestraint.h"
%include "IMP/core/DistanceToSingletonScore.h"
%include "IMP/core/GravityCenterScoreState.h"
%include "IMP/core/GridClosePairsFinder.h"
%include "IMP/core/Harmonic.h"
%include "IMP/core/HarmonicLowerBound.h"
%include "IMP/core/HarmonicUpperBound.h"
%include "IMP/core/HierarchyDecorator.h"
%include "IMP/core/Linear.h"
%include "IMP/core/MaxChangeScoreState.h"
%include "IMP/core/MaximumChangeScoreState.h"
%include "IMP/core/MolecularDynamics.h"
%include "IMP/core/MolecularHierarchyDecorator.h"
%include "IMP/core/MonteCarlo.h"
%include "IMP/core/NameDecorator.h"
%include "IMP/core/NonbondedRestraint.h"
%include "IMP/core/NormalMover.h"
%include "IMP/core/OpenCubicSpline.h"
%include "IMP/core/QuadraticClosePairsFinder.h"
%include "IMP/core/ResidueDecorator.h"
%include "IMP/core/RestraintSet.h"
%include "IMP/core/SingletonListRestraint.h"
%include "IMP/core/SphereDistancePairScore.h"
%include "IMP/core/SteepestDescent.h"
%include "IMP/core/TransformedDistancePairScore.h"
%include "IMP/core/TypedPairScore.h"
%include "IMP/core/VRMLLogOptimizerState.h"
%include "IMP/core/VelocityScalingOptimizerState.h"
%include "IMP/core/XYZRDecorator.h"
%include "IMP/core/model_io.h"
%include "IMP/core/deprecation.h"
%include "IMP/core/PairChainRestraint.h"
%include "IMP/core/PairListRestraint.h"
%include "IMP/core/TripletChainRestraint.h"

%include "IMP/core/ListPairContainer.h"
%include "IMP/core/PairsRestraint.h"
%include "IMP/core/FilteredListSingletonContainer.h"
%include "IMP/core/FilteredListPairContainer.h"
%include "IMP/core/PairsScoreState.h"
%include "IMP/core/SingletonsRestraint.h"
%include "IMP/core/ListSingletonContainer.h"
%include "IMP/core/PairContainerSet.h"
%include "IMP/core/SingletonContainerSet.h"
%include "IMP/core/SingletonsScoreState.h"
%include "IMP/core/MinimumSingletonScoreRestraint.h"
%include "IMP/core/MinimumPairScoreRestraint.h"
%include "IMP/core/MaximumSingletonScoreRestraint.h"
%include "IMP/core/MaximumPairScoreRestraint.h"
%include "IMP/core/Rotation3D.h"
%include "IMP/core/Transformation3D.h"
%include "IMP/core/TransformationFunction.h"




namespace IMP {
  namespace core {
    %template(show_named_hierarchy) show<NameDecorator>;
    %template(show_molecular_hierarchy) show<MolecularHierarchyDecorator>;
    %template(MoverIndex) ::IMP::Index<Mover>;
    %template(BondedListIndex) ::IMP::Index<BondedListScoreState>;
    %template(BondDecorators) ::std::vector<BondDecorator>;
    %template(Movers) ::std::vector<Mover*>;
    %template(SingletonContainers) ::std::vector<SingletonContainer*>;
    %template(XYZDecorators) ::std::vector<XYZDecorator>;
    %template(XYZRDecorators) ::std::vector<XYZRDecorator>;
    %template(PairContainers) ::std::vector<PairContainer*>;
    %template(SingletonContainerIndex) ::IMP::Index<SingletonContainer>;
    %template(PairContainerIndex) ::IMP::Index<PairContainer>;
  }
}
