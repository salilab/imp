#define IMPEXPORT
#define IMPLOCAL


/* Add additional IMP_CONTAINER methods for scripting languages */
%define IMP_CONTAINER_SWIG(type, Ucname, lcname)
%extend type {
  IMP::Ucname##s get_##lcname##s() const {
    IMP::Ucname##s ret(self->lcname##s_begin(), self->lcname##s_end());
    return ret;
  }
}
%enddef

%include "IMP/macros.h"
%include "IMP/container_macros.h"
