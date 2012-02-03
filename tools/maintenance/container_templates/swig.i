/* DO NOT EDIT. Edit modules/core/tools/container-templates/swig.i instead */

namespace IMP {
  namespace core {
    %extend CLASSNAMEContainerSet {
       ::IMP::CLASSNAMEContainers get_FUNCTIONNAME_containers() const {
         return IMP::CLASSNAMEContainers(self->FUNCTIONNAME_containers_begin(),
         self->FUNCTIONNAME_containers_end());
       }
    }
  }
}
