%module(directors="1") "IMP.modulename"

%{
#include "IMP.h"
#include "IMP/core.h"
#include "IMP/algebra.h"
#include "IMP/modulename.h"
%}


%include "IMP_macros.i"
%include "IMP_exceptions.i"

%include "modulename_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

%pythoncode %{
import IMP.core
def get_is_xyz_particle(p):
    """Return True if the particle is an IMP.core.XYZ particle"""
    return IMP.core.XYZ.is_instance_of(p)
%}

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel.i"
%import "core.i"
%import "algebra.i"

namespace IMP {
namespace modulename {
}
}

/* Wrap our own classes */
//%include "IMP/modulename/MyRestraint.h"
