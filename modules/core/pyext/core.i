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

/** This should go back into IMP_macros.i */
%define IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(Ucname)
%pythonprepend Ucname::Ucname %{
        if len(args) >= 1: args[0].thisown=0
        if len(args) >= 2: args[1].thisown=0
%}
%enddef


/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "kernel/pyext/IMP_keys.i"

namespace IMP {
  namespace core {
    %template(AtomTypeBase) ::IMP::KeyBase<IMP_ATOM_TYPE_INDEX>;
    %template(ResidueTypeBase) ::IMP::KeyBase<IMP_RESIDUE_TYPE_INDEX>;

    IMP_OWN_FIRST_CONSTRUCTOR(AngleRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(AngleTripletScore)
    IMP_OWN_FIRST_CONSTRUCTOR(AttributeSingletonScore)
    IMP_OWN_FIRST_CONSTRUCTOR(BondDecoratorRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(BondDecoratorSingletonScore)
    IMP_OWN_FIRST_CONSTRUCTOR(ConnectivityRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(DihedralRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(DistancePairScore)
    IMP_OWN_FIRST_CONSTRUCTOR(DistanceRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(DistanceToSingletonScore)
    IMP_OWN_FIRST_CONSTRUCTOR(NonbondedRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(PairChainRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(PairListRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(SingletonListRestraint)
    IMP_OWN_FIRST_CONSTRUCTOR(SphereDistancePairScore)
    IMP_OWN_FIRST_CONSTRUCTOR(TransformedDistancePairScore)
    IMP_OWN_FIRST_CONSTRUCTOR(TripletChainRestraint)

    /* these two create a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(ParticlesRestraint)
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(ParticlePairsRestraint)

    IMP_CONTAINER_SWIG(RestraintSet, Restraint, restraint)
    IMP_CONTAINER_SWIG(LowestNRestraintSet, Restraint, restraint)
    IMPCORE_CONTAINER_SWIG(MonteCarlo, Mover, mover)
    IMPCORE_CONTAINER_SWIG(FilteredListParticleContainer,
    ParticleContainer, particle_container)
    IMPCORE_CONTAINER_SWIG(FilteredListParticlePairContainer,
    ParticlePairContainer, particle_pair_container)

    IMP_SET_OBJECT(MonteCarlo, set_local_optimizer)
    IMP_SET_OBJECT(TypedPairScore, set_pair_score)


    IMP_ADD_OBJECT(NonbondedListScoreState, add_bonded_list)
    IMP_ADD_OBJECTS(NonbondedListScoreState, add_bonded_lists)
  }
}

/* Don't wrap internal classes */
%ignore IMP::core::internal::ChildArrayTraits;

/* Wrap our own base classes */
%include "IMP/core/ClosePairsFinder.h"
%include "IMP/core/Mover.h"
%include "IMP/core/MoverBase.h"
%include "IMP/core/ParticleContainer.h"
%include "IMP/core/ParticlePairContainer.h"
%include "IMP/core/XYZDecorator.h"

%include "IMP/core/BondedListScoreState.h"
%include "IMP/core/NonbondedListScoreState.h"

/* Must be included before HierarchyDecorator.h since it is not easy
   to predeclare a typedef (for BondDecorators) */ 
%include "IMP/core/bond_decorators.h"


/* Wrap the final classes */
%include "IMP/core/AllNonbondedListScoreState.h"
%include "IMP/core/AngleRestraint.h"
%include "IMP/core/AngleTripletScore.h"
%include "IMP/core/AtomDecorator.h"
%include "IMP/core/AttributeSingletonScore.h"
%include "IMP/core/BallMover.h"
%include "IMP/core/BipartiteNonbondedListScoreState.h"
%include "IMP/core/BondDecoratorListScoreState.h"
%include "IMP/core/BondDecoratorParticlePairContainer.h"
%include "IMP/core/BondDecoratorRestraint.h"
%include "IMP/core/BondDecoratorSingletonScore.h"
%include "IMP/core/BrownianDynamics.h"
%include "IMP/core/CMMLogOptimizerState.h"
%include "IMP/core/ClosedCubicSpline.h"
%include "IMP/core/ConjugateGradients.h"
%include "IMP/core/ConnectivityRestraint.h"
%include "IMP/core/ConstantRestraint.h"
%include "IMP/core/Cosine.h"
%include "IMP/core/DihedralRestraint.h"
%include "IMP/core/DistancePairScore.h"
%include "IMP/core/DistanceRestraint.h"
%include "IMP/core/DistanceToSingletonScore.h"
%include "IMP/core/FilteredListParticleContainer.h"
%include "IMP/core/FilteredListParticlePairContainer.h"
%include "IMP/core/GravityCenterScoreState.h"
%include "IMP/core/Harmonic.h"
%include "IMP/core/HarmonicLowerBound.h"
%include "IMP/core/HarmonicUpperBound.h"
%include "IMP/core/HierarchyDecorator.h"
%include "IMP/core/Linear.h"
%include "IMP/core/ListParticleContainer.h"
%include "IMP/core/ListParticlePairContainer.h"
%include "IMP/core/LowestNRestraintSet.h"
%include "IMP/core/MaxChangeScoreState.h"
%include "IMP/core/MolecularDynamics.h"
%include "IMP/core/MolecularHierarchyDecorator.h"
%include "IMP/core/MonteCarlo.h"
%include "IMP/core/NameDecorator.h"
%include "IMP/core/NonbondedRestraint.h"
%include "IMP/core/NormalMover.h"
%include "IMP/core/OpenCubicSpline.h"
%include "IMP/core/PairChainRestraint.h"
%include "IMP/core/PairListRestraint.h"
%include "IMP/core/ParticlePairsRestraint.h"
%include "IMP/core/ParticlesRestraint.h"
%include "IMP/core/QuadraticClosePairsFinder.h"
%include "IMP/core/ResidueDecorator.h"
%include "IMP/core/RestraintSet.h"
%include "IMP/core/SingletonListRestraint.h"
%include "IMP/core/SphereDistancePairScore.h"
%include "IMP/core/SteepestDescent.h"
%include "IMP/core/TransformedDistancePairScore.h"
%include "IMP/core/TripletChainRestraint.h"
%include "IMP/core/TypedPairScore.h"
%include "IMP/core/VRMLLogOptimizerState.h"
%include "IMP/core/VelocityScalingOptimizerState.h"
%include "IMP/core/XYZRDecorator.h"
%include "IMP/core/model_io.h"


namespace IMP {
  namespace core {
    %template(show_named_hierarchy) show<NameDecorator>;
    %template(show_molecular_hierarchy) show<MolecularHierarchyDecorator>;
    %template(MoverIndex) ::IMP::Index<Mover>;
    %template(BondedListIndex) ::IMP::Index<BondedListScoreState>;
    %template(BondDecorators) ::std::vector<BondDecorator>;
    %template(Movers) ::std::vector<Mover*>;
    %template(ParticleContainers) ::std::vector<ParticleContainer*>;
    %template(ParticlePairContainers) ::std::vector<ParticlePairContainer*>;
    %template(ParticleContainerIndex) ::IMP::Index<ParticleContainer>;
    %template(ParticlePairContainerIndex) ::IMP::Index<ParticlePairContainer>;
  }
}
