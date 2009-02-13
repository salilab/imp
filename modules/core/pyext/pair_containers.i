namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMP_OWN_CONSTRUCTOR(PairsRestraint)
    IMP_OWN_CONSTRUCTOR(PairsScoreState)
    IMP_OWN_CONSTRUCTOR(MinimumPairScoreRestraint)
    IMP_OWN_CONSTRUCTOR(MaximumPairScoreRestraint)
    IMP_CONTAINER_SWIG(FilteredListPairContainer, PairContainer, pair_container)
    IMP_SET_OBJECT(PairsRestraint, set_pair_container)
    IMP_SET_OBJECT(PairsScoreState, set_before_evaluate_modifier)
    IMP_SET_OBJECT(PairsScoreState, set_after_evaluate_modifier)
    /*IMP_CONTAINER_SWIG(PairContainerSet, PairContainer, pair_container)
    IMP_CONTAINER_SWIG(ListPairContainer, Pair, pair)*/
    IMP_ADD_OBJECT(ListPairContainer, add_pair)
    IMP_ADD_OBJECTS(ListPairContainer, add_pairs)
    IMP_ADD_OBJECT(PairContainerSet, add_pair_container)
    IMP_ADD_OBJECTS(PairContainerSet, add_pair_containers)

    %extend ListPairContainer {
       ParticlePairs get_particle_pairs() const {
         return IMP::ParticlePairs(self->particle_pairs_begin(), self->particle_pairs_end());
       }
    }
    %extend PairContainerSet {
       PairContainers get_pair_containers() const {
         return IMP::PairContainers(self->pair_containers_begin(),
         self->pair_containers_end());
       }
    }

    IMP_ADD_OBJECT(FilteredListPairContainer, add_pair)
    IMP_ADD_OBJECTS(FilteredListPairContainer, add_pairs)
    %extend FilteredListPairContainer {
       ParticlePairs get_particle_pairs() const {
         return IMP::ParticlePairs(self->particle_pairs_begin(), self->particle_pairs_end());
       }
    }
  }
}
