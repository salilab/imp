%module(directors="1") "IMP.benchmark"

%{
#include "IMP.h"
#include "IMP/core.h"
#include "IMP/algebra.h"
#include "IMP/benchmark.h"
%}


%include "IMP_macros.i"
%include "IMP_exceptions.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"


/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel.i"
%import "core.i"
%import "algebra.i"

%include "IMP/benchmark/config.h"

/* Wrap our own classes */
//%include "IMP/benchmark/MyRestraint.h"
