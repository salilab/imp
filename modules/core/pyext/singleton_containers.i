namespace IMP {
  namespace core {

    IMP_CONTAINER_SWIG(FilteredListSingletonContainer, SingletonFilter, singleton_filter)
    /*IMP_CONTAINER_SWIG(SingletonContainerSet, SingletonContainer, singleton_container)
    IMP_CONTAINER_SWIG(ListSingletonContainer, Singleton, singleton)*/

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

    %extend FilteredListSingletonContainer {
       Particles get_particles() const {
         return IMP::Particles(self->particles_begin(), self->particles_end());
       }
    }
  }
}
