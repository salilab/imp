/* DO NOT EDIT. Edit modules/core/tools/container-templates/swig.i instead */

namespace IMP {
  namespace core {
    %extend ListSingletonContainer {
       Particles get_particles() const {
         return IMP::Particles(self->particles_begin(), self->particles_end());
       }
    }
    %extend SingletonContainerSet {
       SingletonContainers get_singleton_containers() const {
         return IMP::SingletonContainers(self->singleton_containers_begin(),
         self->singleton_containers_end());
       }
    }
  }
}
