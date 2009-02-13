namespace IMP {
  namespace core {

    /* this creates a memory leak.
       We need to check if the object inherits from ref counted or
       object first */
    IMP_OWN_CONSTRUCTOR(GroupnamesRestraint)
    IMP_OWN_CONSTRUCTOR(GroupnamesScoreState)
    IMP_OWN_CONSTRUCTOR(MinimumGroupnameScoreRestraint)
    IMP_OWN_CONSTRUCTOR(MaximumGroupnameScoreRestraint)
    IMP_CONTAINER_SWIG(FilteredListGroupnameContainer, GroupnameContainer, groupname_container)
    IMP_SET_OBJECT(GroupnamesRestraint, set_groupname_container)
    IMP_SET_OBJECT(GroupnamesScoreState, set_before_evaluate_modifier)
    IMP_SET_OBJECT(GroupnamesScoreState, set_after_evaluate_modifier)
    /*IMP_CONTAINER_SWIG(GroupnameContainerSet, GroupnameContainer, groupname_container)
    IMP_CONTAINER_SWIG(ListGroupnameContainer, Groupname, groupname)*/
    IMP_ADD_OBJECT(ListGroupnameContainer, add_groupname)
    IMP_ADD_OBJECTS(ListGroupnameContainer, add_groupnames)
    IMP_ADD_OBJECT(GroupnameContainerSet, add_groupname_container)
    IMP_ADD_OBJECTS(GroupnameContainerSet, add_groupname_containers)

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

    IMP_ADD_OBJECT(FilteredListGroupnameContainer, add_groupname)
    IMP_ADD_OBJECTS(FilteredListGroupnameContainer, add_groupnames)
    %extend FilteredListGroupnameContainer {
       Classnames get_classnames() const {
         return IMP::Classnames(self->classnames_begin(), self->classnames_end());
       }
    }
  }
}
