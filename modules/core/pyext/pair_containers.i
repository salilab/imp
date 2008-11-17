namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(PairsRestraint)
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(PairsScoreState)
    IMP_CONTAINER_SWIG(ListPairContainer, ParticlePair, particle_pair)
    IMP_CONTAINER_SWIG(FilteredListPairContainer, ParticlePair, particle_pair)
    IMP_SET_OBJECT(PairsRestraint, set_pair_container)
    IMPCORE_CONTAINER_SWIG(PairContainerSet, PairContainer, pair_container)

    }
}
