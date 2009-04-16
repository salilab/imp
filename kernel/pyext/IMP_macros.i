#define IMPEXPORT
#define IMPLOCAL

#define IMP_BEGIN_NAMESPACE \
namespace IMP \
{

#define IMP_END_NAMESPACE \
} /* namespace IMP */

%pythoncode %{
import IMP
def disown_ref_counted(p):
    """Set p.thisown to False if p is ref counted"""
    import IMP
    if p is not None and issubclass(type(p), IMP.RefCounted):
      p.thisown=0;
%}

/* Using this introduces a spurious end C comment token into the code.
   So just copy and paste the code. */
%define IMP_FIND_FIRST_ARGUMENT()
        # swig occasionally changes whether self is defined or not
        # we need to make sure we skip it and nothing more
        imp_first_argument_index=1
        try:
            str(self)
        except NameError:
            pass
        else:
            imp_first_argument_index=0
%enddef

%define IMP_OWN_CONSTRUCTOR(Ucname)
%pythonprepend Ucname::Ucname %{
        import IMP
        imp_first_argument_index=1
        try:
            str(self)
        except NameError:
            pass
        else:
            imp_first_argument_index=0
        for i in range(imp_first_argument_index, len(args)):
            IMP.disown_ref_counted(args[i])
%}
%enddef

%define IMP_OWN_METHOD(Ucname, methodname)
%pythonprepend Ucname::methodname %{
        import IMP
        imp_first_argument_index=1
        try:
            str(self)
        except NameError:
            pass
        else:
            imp_first_argument_index=0
        for i in range(imp_first_argument_index, len(args)):
            IMP.disown_ref_counted(args[i])
%}
%enddef

%define IMP_OWN_FUNCTION(functionname)
%pythonprepend functionname %{
  import IMP
  for i in range(0, len(args)):
      IMP.disown_ref_counted(args[i])
%}
%pythonappend functionname %{
  IMP.disown_ref_counted(val)
%}
%enddef

%define IMP_OWN_LIST_METHOD(Ucname, methodname)
%pythonprepend Ucname::methodname %{
        import IMP
        imp_first_argument_index=1
        try:
            str(self)
        except NameError:
            pass
        else:
            imp_first_argument_index=0
        for p in args[imp_first_argument_index]:
            IMP.disown_ref_counted(p)
%}
%enddef

/* Add additional IMP_CONTAINER methods for scripting languages */
%define IMP_CONTAINER_SWIG(type, Ucname, lcname)
%extend type {
  IMP::Ucname##s get_##lcname##s() const {
    IMP::Ucname##s ret(self->lcname##s_begin(), self->lcname##s_end());
    return ret;
  }
}
IMP_OWN_METHOD(type, add_##lcname)
IMP_OWN_LIST_METHOD(type, add_##lcname##s)
%enddef

%include "IMP/macros.h"
%include "IMP/container_macros.h"
%include "IMP/internal/config.h"
