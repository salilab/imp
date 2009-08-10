%module(directors="1") "IMP.helper"

%{
#include "IMP.h"
#include "IMP/core.h"
#include "IMP/algebra.h"
#include "IMP/helper.h"
#include "IMP/atom.h"
%}


%include "IMP_macros.i"
%include "IMP_exceptions.i"
%include "IMP_decorators.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel.i"
%import "core.i"
%import "algebra.i"
%import "atom.i"

%include "IMP/helper/config.h"

namespace IMP {
namespace helper {
}
}

/* Wrap our own classes */
%include "IMP/helper/rigid_bodies.h"
%include "IMP/helper/covers.h"
%include "IMP/helper/atom_hierarchy.h"
