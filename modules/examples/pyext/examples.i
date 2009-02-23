%module(directors="1") "IMP.examples"

%{
#include "IMP.h"
#include "IMP/examples.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
%}


%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "examples_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"


/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "modules/core/pyext/core.i"
%import "modules/algebra/pyext/algebra.i"

namespace IMP {
namespace examples {
IMP_OWN_CONSTRUCTOR(Restraint)
}
}

/* Wrap our own classes */
%include "IMP/examples/ExampleRestraint.h"
%include "IMP/examples/ExampleDecorator.h"
%include "IMP/examples/ExampleUnaryFunction.h"
