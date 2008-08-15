%module(directors="1") DOMINO

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
#define DOMINODLLEXPORT
#define DOMINODLLLOCAL

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

// generate directors for all classes that have virtual methods
%feature("director");
%feature("director") DiscreteSampler;
%feature("director") DiscreteSet;

namespace IMP {
  typedef float Float;
  typedef int Int;
  class FloatKey;
  class Particle;
  class Particles;
  class DerivativeAccumulator;
}
/* Get definitions of IMP base classes (but do not wrap; that is done by IMP) */
%import "IMP/VersionInfo.h"
%import "IMP/Restraint.h"
%import "IMP/Object.h"
%import "IMP/restraints/RestraintSet.h"
%import "IMP/DerivativeAccumulator.h"


/* Wrap our own classes */
%include "../src/DiscreteSampler.h"
%include "../src/RestraintGraph.h"
%include "../src/OptimizationWorkFlow.h"
%include "../src/SimpleDiscreteSpace.h"
%include "../src/DominoOptimizer.h"
/*%include "../src/DominoMCOptimizer.h"*/
%include "../src/SimpleDiscreteRestraint.h"
