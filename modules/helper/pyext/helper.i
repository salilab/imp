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
%include "modules/helper/pyext/helper_config.i"

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
%import "atom.i"

namespace IMP {
namespace helper {
}
}

/* Wrap our own classes */
%include "IMP/helper/rigid_bodies.h"
%include "IMP/helper/covers.h"
%include "IMP/helper/atom_hierarchy.h"
