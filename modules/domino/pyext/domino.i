%module(directors="1") IMP_domino

%{
#include "IMP.h"
#include "IMP/domino.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "domino_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

// generate directors for all classes that have virtual methods
%feature("director") IMP::domino::DiscreteSampler;

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"

/* Wrap our own classes */
%include "IMP/domino/DiscreteSampler.h"
%include "IMP/domino/CombState.h"
%include "IMP/domino/RestraintGraph.h"
%include "IMP/domino/SimpleDiscreteSpace.h"
%include "IMP/domino/DominoOptimizer.h"
%include "IMP/domino/SimpleDiscreteRestraint.h"

