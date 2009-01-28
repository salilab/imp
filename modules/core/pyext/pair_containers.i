namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMP_OWN_FIRST_SECOND_CONSTRUCTOR(PairsRestraint)
    IMP_OWN_FIRST_SECOND_THIRD_CONSTRUCTOR(PairsScoreState)
    IMP_OWN_FIRST_SECOND_CONSTRUCTOR(MinimumPairScoreRestraint)
    IMP_OWN_FIRST_SECOND_CONSTRUCTOR(MaximumPairScoreRestraint)
    IMP_CONTAINER_SWIG(FilteredListPairContainer, PairContainer, pair_container)
    IMP_SET_OBJECT(PairsRestraint, set_pair_container)
    IMP_CONTAINER_SWIG(ListPairContainer, ParticlePair, particle_pair)
    IMP_SET_OBJECT(PairsScoreState, set_before_evaluate_modifier)
    IMP_SET_OBJECT(PairsScoreState, set_after_evaluate_modifier)
    IMP_CONTAINER_SWIG(PairContainerSet, PairContainer, pair_container)

    }
}
