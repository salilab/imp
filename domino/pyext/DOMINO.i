%module(directors="1") IMP_domino

%{
#include "IMP.h"
#include "../src/DiscreteSampler.h"
#include "../src/RestraintGraph.h"
#include "../src/OptimizationWorkFlow.h"
#include "../src/SimpleDiscreteSpace.h"
#include "../src/DominoOptimizer.h"
/*#include "../src/DominoMCOptimizer.h"*/
#include "../src/SimpleDiscreteRestraint.h"
%}

%include "kernel/pyext/IMP_macros.i"

/* Ignore shared object import/export stuff */
#define IMPDOMINOEXPORT
#define IMPDOMINOLOCAL

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

// generate directors for all classes that have virtual methods
%feature("director");
%feature("director") DiscreteSampler;
%feature("director") DiscreteSet;

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"

/* Don't use the exception handlers defined in the kernel */
%exception;

/* Wrap our own classes */
%include "../src/DiscreteSampler.h"
%include "../src/RestraintGraph.h"
%include "../src/OptimizationWorkFlow.h"
%include "../src/SimpleDiscreteSpace.h"
%include "../src/DominoOptimizer.h"
/*%include "../src/DominoMCOptimizer.h"*/
%include "../src/SimpleDiscreteRestraint.h"
