#define IMPEXPORT
#define IMPLOCAL

#define IMP_BEGIN_NAMESPACE \
namespace IMP \
{

#define IMP_END_NAMESPACE \
} /* namespace IMP */

%define IMP_OWN_CONSTRUCTOR(Ucname)
%pythonprepend Ucname::Ucname %{
        import IMP
        for i in range(0, len(args)):
            if args[i] is not None and issubclass(type(args[i]), IMP.RefCountedObject):
               args[i].thisown=0;
%}
%enddef

%define IMP_OWN_METHOD(Ucname, methodname)
%pythonprepend Ucname::methodname %{
        import IMP
        for i in range(1, len(args)):
            if args[i] is not None and issubclass(type(args[i]), IMP.RefCountedObject):
               args[i].thisown=0;
%}
%enddef

%define IMP_OWN_FUNCTION(functionname)
%pythonprepend functionname %{
  import IMP
  for i in range(0, len(args)):
    if args[i] is not None and issubclass(type(args[i]), IMP.RefCountedObject):
       args[i].thisown=0;
%}
%pythonappend functionname %{
  if val is not None and issubclass(type(val), IMP.RefCountedObject):
     val.thisown=0;
%}
%enddef

%define IMP_OWN_LIST_METHOD(Ucname, methodname)
%pythonprepend Ucname::methodname %{
        import IMP
        for p in args[1]:
            if p is not None and issubclass(type(p), IMP.RefCountedObject):
               p.thisown=0;
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
