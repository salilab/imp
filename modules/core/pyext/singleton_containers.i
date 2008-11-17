namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(SingletonsRestraint)
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(SingletonsScoreState)
    IMPCORE_CONTAINER_SWIG(FilteredListSingletonContainer, SingletonContainer, singleton_container)
    IMP_SET_OBJECT(SingletonsRestraint, set_singleton_container)
    IMPCORE_CONTAINER_SWIG(SingletonContainerSet, SingletonContainer, singleton_container)

    }
}
