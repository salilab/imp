namespace IMP {
  namespace core {

    IMP_CONTAINER_SWIG(FilteredListPairContainer, PairFilter, pair_filter)
    /*IMP_CONTAINER_SWIG(PairContainerSet, PairContainer, pair_container)
    IMP_CONTAINER_SWIG(ListPairContainer, Pair, pair)*/

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

    %extend FilteredListPairContainer {
       ParticlePairs get_particle_pairs() const {
         return IMP::ParticlePairs(self->particle_pairs_begin(), self->particle_pairs_end());
       }
    }
  }
}
