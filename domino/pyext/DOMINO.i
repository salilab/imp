%module(directors="1") IMP_domino

%{
#include "IMP.h"
#include "IMP/domino/DiscreteSampler.h"
#include "IMP/domino/CombState.h"
#include "IMP/domino/RestraintGraph.h"
#include "IMP/domino/SimpleDiscreteSpace.h"
#include "IMP/domino/DominoOptimizer.h"
#include "IMP/domino/SimpleDiscreteRestraint.h"
%}

%include "kernel/pyext/IMP_macros.i"

/* Ignore shared object import/export stuff */
#define DOMINODLLEXPORT
#define DOMINODLLLOCAL
#define IMPDOMINOEXPORT
#define IMPDOMINOLOCAL

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

// generate directors for all classes that have virtual methods
%feature("director") IMP::domino::DiscreteSampler;

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"

/* Don't use the exception handlers defined in the kernel */
%exception;

/* Wrap our own classes */
%include "IMP/domino/DiscreteSampler.h"
%include "IMP/domino/CombState.h"
%include "IMP/domino/RestraintGraph.h"
%include "IMP/domino/SimpleDiscreteSpace.h"
%include "IMP/domino/DominoOptimizer.h"
%include "IMP/domino/SimpleDiscreteRestraint.h"

