%module(directors="1") "IMP.examples"

%{
#include "IMP.h"
#include "IMP/examples.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
%}


%include "IMP_macros.i"
%include "IMP_exceptions.i"
%include "IMP_streams.i"
%include "IMP_decorators.i"

%include "modules/examples/pyext/examples_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"


/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel.i"
%import "core.i"
%import "algebra.i"

/* Wrap our own classes */
%include "IMP/examples/ExampleRestraint.h"
%include "IMP/examples/ExampleDecorator.h"
%include "IMP/examples/ExampleUnaryFunction.h"
%include "IMP/examples/ExampleRefCounted.h"

namespace IMP {
  namespace examples {
   IMP_DECORATORS(ExampleDecorator, ExampleDecorators, Particles)
  }
}