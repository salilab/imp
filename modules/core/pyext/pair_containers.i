/* DO NOT EDIT. Edit modules/core/tools/container-templates/swig.i instead */

namespace IMP {
  namespace core {
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
  }
}
