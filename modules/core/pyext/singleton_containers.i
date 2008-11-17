namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(SingletonsRestraint)
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(SingletonsScoreState)
    IMP_CONTAINER_SWIG(ListSingletonContainer, Particle, particle)
    IMP_CONTAINER_SWIG(FilteredListSingletonContainer, Particle, particle)
    IMP_SET_OBJECT(SingletonsRestraint, set_singleton_container)
    IMPCORE_CONTAINER_SWIG(SingletonContainerSet, SingletonContainer, singleton_container)

    }
}
