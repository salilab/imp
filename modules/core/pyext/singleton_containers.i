namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMP_OWN_FIRST_SECOND_CONSTRUCTOR(SingletonsRestraint)
    IMP_OWN_FIRST_SECOND_THIRD_CONSTRUCTOR(SingletonsScoreState)
    IMP_OWN_FIRST_SECOND_CONSTRUCTOR(MinimumSingletonScoreRestraint)
    IMP_OWN_FIRST_SECOND_CONSTRUCTOR(MaximumSingletonScoreRestraint)
    IMP_CONTAINER_SWIG(FilteredListSingletonContainer, SingletonContainer, singleton_container)
    IMP_SET_OBJECT(SingletonsRestraint, set_singleton_container)
    IMP_SET_OBJECT(SingletonsScoreState, set_before_evaluate_modifier)
    IMP_SET_OBJECT(SingletonsScoreState, set_after_evaluate_modifier)
    /*IMP_CONTAINER_SWIG(SingletonContainerSet, SingletonContainer, singleton_container)
    IMP_CONTAINER_SWIG(ListSingletonContainer, Singleton, singleton)*/
    IMP_ADD_OBJECT(ListSingletonContainer, add_singleton)
    IMP_ADD_OBJECTS(ListSingletonContainer, add_singletons)
    IMP_ADD_OBJECT(SingletonContainerSet, add_singleton_container)
    IMP_ADD_OBJECTS(SingletonContainerSet, add_singleton_containers)

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

    IMP_ADD_OBJECT(FilteredListSingletonContainer, add_singleton)
    IMP_ADD_OBJECTS(FilteredListSingletonContainer, add_singletons)
    %extend FilteredListSingletonContainer {
       Particles get_particles() const {
         return IMP::Particles(self->particles_begin(), self->particles_end());
       }
    }
  }
}
