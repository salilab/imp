/* DO NOT EDIT. Edit modules/core/tools/container-templates/swig.i instead */

namespace IMP {
  namespace core {
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
  }
}
