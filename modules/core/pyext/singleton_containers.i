/* DO NOT EDIT. Edit modules/core/tools/container-templates/swig.i instead */

namespace IMP {
  namespace core {
    %extend SingletonContainerSet {
       IMP::SingletonContainers get_singleton_containers() const {
         return IMP::SingletonContainers(self->singleton_containers_begin(),
         self->singleton_containers_end());
       }
    }
  }
}
