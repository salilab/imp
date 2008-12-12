%module(directors="1") IMP_modulename

%{
#include "IMP.h"
#include "IMP/modulename.h"
%}


%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "modulename_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "kernel/pyext/IMP_keys.i"

namespace IMP {
  typedef VectorD<3> Vector3D;
}

// turn off python memory management for RefCountedObjects
namespace IMP {
  namespace modulename {
    IMP_OWN_FIRST_CONSTRUCTOR(MyRestraint)
  }
}
/* Wrap our own classes */
%include "IMP/modulename/MyRestraint.h"
