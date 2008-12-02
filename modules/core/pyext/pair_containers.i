namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(PairsRestraint)
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(PairsScoreState)
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(MinimumPairScoreRestraint)
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(MaximumPairScoreRestraint)
    IMPCORE_CONTAINER_SWIG(FilteredListPairContainer, PairContainer, pair_container)
    IMP_SET_OBJECT(PairsRestraint, set_pair_container)
    IMP_SET_OBJECT(PairsScoreState, set_before_evaluate_modifier)
    IMP_SET_OBJECT(PairsScoreState, set_after_evaluate_modifier)
    IMPCORE_CONTAINER_SWIG(PairContainerSet, PairContainer, pair_container)

    }
}
