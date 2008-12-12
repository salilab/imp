#define IMPEXPORT
#define IMPLOCAL

#define IMP_BEGIN_NAMESPACE \
namespace IMP \
{

#define IMP_END_NAMESPACE \
} /* namespace IMP */

%define IMP_OWN_FIRST_CONSTRUCTOR(Ucname)
%pythonprepend Ucname::Ucname %{
        if len(args) >= 1 and args[0] is not None: args[0].thisown=0
%}
%enddef

%define IMP_OWN_FIRST_SECOND_CONSTRUCTOR(Ucname)
%pythonprepend Ucname::Ucname %{
        if len(args) >= 1 and args[0] is not None: args[0].thisown=0
        if len(args) >= 2 and args[1] is not None: args[1].thisown=0
%}
%enddef

%define IMP_OWN_FIRST_SECOND_THIRD_CONSTRUCTOR(Ucname)
%pythonprepend Ucname::Ucname %{
        if len(args) >= 1 and args[0] is not None: args[0].thisown=0
        if len(args) >= 2 and args[1] is not None: args[1].thisown=0
        if len(args) >= 3 and args[2] is not None: args[2].thisown=0
%}
%enddef

%define IMP_SET_OBJECT(Ucname, methodname)
%pythonprepend Ucname::methodname %{
        args[1].thisown=0
%}
%enddef

%define IMP_ADD_OBJECT(Ucname, methodname)
%pythonprepend Ucname::methodname %{
        args[1].thisown=0
%}
%enddef

%define IMP_ADD_OBJECTS(Ucname, methodname)
%pythonprepend Ucname::methodname %{
        for p in args[1]:
            p.thisown=0
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
IMP_ADD_OBJECT(type, add_##lcname)
IMP_ADD_OBJECTS(type, add_##lcname##s)
%enddef

%include "IMP/macros.h"
%include "IMP/container_macros.h"
