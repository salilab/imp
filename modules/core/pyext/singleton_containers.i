namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMP_OWN_CONSTRUCTOR(SingletonsRestraint)
    IMP_OWN_CONSTRUCTOR(SingletonRestraint)
    IMP_OWN_CONSTRUCTOR(SingletonsScoreState)
    IMP_OWN_CONSTRUCTOR(SingletonScoreState)
    IMP_OWN_CONSTRUCTOR(MinimumSingletonScoreRestraint)
    IMP_OWN_CONSTRUCTOR(MaximumSingletonScoreRestraint)
    IMP_CONTAINER_SWIG(FilteredListSingletonContainer, SingletonFilter, singleton_filter)
    IMP_OWN_METHOD(SingletonsRestraint, set_singleton_container)
    IMP_OWN_METHOD(SingletonsScoreState, set_before_evaluate_modifier)
    IMP_OWN_METHOD(SingletonsScoreState, set_after_evaluate_modifier)
    /*IMP_CONTAINER_SWIG(SingletonContainerSet, SingletonContainer, singleton_container)
    IMP_CONTAINER_SWIG(ListSingletonContainer, Singleton, singleton)*/
    IMP_OWN_METHOD(ListSingletonContainer, add_singleton)
    IMP_OWN_LIST_METHOD(ListSingletonContainer, add_singletons)
    IMP_OWN_METHOD(SingletonContainerSet, add_singleton_container)
    IMP_OWN_LIST_METHOD(SingletonContainerSet, add_singleton_containers)

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

    IMP_OWN_METHOD(FilteredListSingletonContainer, add_singleton)
    IMP_OWN_LIST_METHOD(FilteredListSingletonContainer, add_singletons)
    %extend FilteredListSingletonContainer {
       Particles get_particles() const {
         return IMP::Particles(self->particles_begin(), self->particles_end());
       }
    }
  }
}
