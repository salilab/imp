/* DO NOT EDIT. Edit modules/core/tools/container-templates/swig.i instead */

namespace IMP {
  namespace core {
    IMP_CONTAINER_SWIG(FilteredListGroupnameContainer, GroupnameFilter, groupname_filter)
    /*IMP_CONTAINER_SWIG(GroupnameContainerSet, GroupnameContainer, groupname_container)
    IMP_CONTAINER_SWIG(ListGroupnameContainer, Groupname, groupname)*/

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

    %extend FilteredListGroupnameContainer {
       Classnames get_classnames() const {
         return IMP::Classnames(self->classnames_begin(), self->classnames_end());
       }
    }
  }
}
