namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(GroupnamesRestraint)
    IMPCORE_OWN_FIRST_SECOND_THIRD_CONSTRUCTOR(GroupnamesScoreState)
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(MinimumGroupnameScoreRestraint)
    IMPCORE_OWN_FIRST_SECOND_CONSTRUCTOR(MaximumGroupnameScoreRestraint)
    IMPCORE_CONTAINER_SWIG(FilteredListGroupnameContainer, GroupnameContainer, groupname_container)
    IMP_SET_OBJECT(GroupnamesRestraint, set_groupname_container)
    IMP_SET_OBJECT(GroupnamesScoreState, set_before_evaluate_modifier)
    IMP_SET_OBJECT(GroupnamesScoreState, set_after_evaluate_modifier)
    IMPCORE_CONTAINER_SWIG(GroupnameContainerSet, GroupnameContainer, groupname_container)

    }
}
