/* DO NOT EDIT. Edit modules/core/tools/container-templates/swig.i instead */

namespace IMP {
  namespace core {
    %extend PairContainerSet {
       PairContainers get_pair_containers() const {
         return IMP::PairContainers(self->pair_containers_begin(),
         self->pair_containers_end());
       }
    }
  }
}
