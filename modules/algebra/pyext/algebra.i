%module(directors="1") "IMP.algebra"

%{
#include "IMP.h"
#include "IMP/algebra.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "algebra_config.i"
%include "exception.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "kernel/pyext/IMP_keys.i"

namespace IMP {
  typedef VectorD<3> Vector3D;
}

/* Wrap our own base classes */
%include "IMP/algebra/Rotation3D.h"
%include "IMP/algebra/Transformation3D.h"
%include "IMP/algebra/geometric_alignment.h"
