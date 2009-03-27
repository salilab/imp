namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMP_OWN_CONSTRUCTOR(GroupnamesRestraint)
    IMP_OWN_CONSTRUCTOR(GroupnameRestraint)
    IMP_OWN_CONSTRUCTOR(GroupnamesScoreState)
    IMP_OWN_CONSTRUCTOR(GroupnameScoreState)
    IMP_OWN_CONSTRUCTOR(MinimumGroupnameScoreRestraint)
    IMP_OWN_CONSTRUCTOR(MaximumGroupnameScoreRestraint)
    IMP_CONTAINER_SWIG(FilteredListGroupnameContainer, GroupnameFilter, groupname_filter)
    IMP_OWN_METHOD(GroupnamesRestraint, set_groupname_container)
    IMP_OWN_METHOD(GroupnamesScoreState, set_before_evaluate_modifier)
    IMP_OWN_METHOD(GroupnamesScoreState, set_after_evaluate_modifier)
    /*IMP_CONTAINER_SWIG(GroupnameContainerSet, GroupnameContainer, groupname_container)
    IMP_CONTAINER_SWIG(ListGroupnameContainer, Groupname, groupname)*/
    IMP_OWN_METHOD(ListGroupnameContainer, add_groupname)
    IMP_OWN_LIST_METHOD(ListGroupnameContainer, add_groupnames)
    IMP_OWN_METHOD(GroupnameContainerSet, add_groupname_container)
    IMP_OWN_LIST_METHOD(GroupnameContainerSet, add_groupname_containers)

    %extend ListGroupnameContainer {
       Classnames get_classnames() const {
         return IMP::Classnames(self->classnames_begin(), self->classnames_end());
       }
    }
    %extend GroupnameContainerSet {
       GroupnameContainers get_groupname_containers() const {
         return IMP::GroupnameContainers(self->groupname_containers_begin(),
         self->groupname_containers_end());
       }
    }

    IMP_OWN_METHOD(FilteredListGroupnameContainer, add_groupname)
    IMP_OWN_LIST_METHOD(FilteredListGroupnameContainer, add_groupnames)
    %extend FilteredListGroupnameContainer {
       Classnames get_classnames() const {
         return IMP::Classnames(self->classnames_begin(), self->classnames_end());
       }
    }
  }
}
