%module(directors="1") "IMP.examples"

%{
#include "IMP.h"
#include "IMP/examples.h"
%}


%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "examples_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

%pythoncode %{
import IMP.core
def get_is_xyz_particle(p):
    """Return True if the particle is an IMP.core.XYZDecorator particle"""
    return IMP.core.XYZDecorator.is_instance_of(p)
%}

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"

namespace IMP {
namespace examples {
IMP_OWN_FIRST_SECOND_CONSTRUCTOR(Restraint)
}
}

/* Wrap our own classes */
%include "IMP/examples/ExampleRestraint.h"
%include "IMP/examples/ExampleDecorator.h"
%include "IMP/examples/ExampleUnaryFunction.h"
