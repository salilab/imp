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
    IMP_CONTAINER_SWIG(ListSingletonContainer, Particle, particle)
    IMP_SET_OBJECT(SingletonsRestraint, set_singleton_container)
    IMP_SET_OBJECT(SingletonsScoreState, set_before_evaluate_modifier)
    IMP_SET_OBJECT(SingletonsScoreState, set_after_evaluate_modifier)
    IMP_CONTAINER_SWIG(SingletonContainerSet, SingletonContainer, singleton_container)

    }
}
