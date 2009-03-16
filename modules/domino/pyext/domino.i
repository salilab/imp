%module(directors="1") "IMP.domino"

%{
#include "IMP.h"
#include "IMP/domino.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "domino_config.i"
%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

// generate directors for all classes that have virtual methods
%feature("director") IMP::domino::DiscreteSampler;
%feature("director") IMP::domino::DiscreteSet;

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "modules/algebra/pyext/algebra.i"
/* Wrap our own classes */
%include "IMP/domino/CombState.h"
%include "IMP/domino/DiscreteSampler.h"
%include "IMP/domino/JNode.h"
%include "IMP/domino/RestraintGraph.h"
%include "IMP/domino/DominoOptimizer.h"
%include "IMP/domino/DiscreteSet.h"
%include "IMP/domino/PermutationSampler.h"
%include "IMP/domino/MappedDiscreteSet.h"
%include "IMP/domino/CartesianProductSampler.h"
%include "IMP/domino/SimpleDiscreteSpace.h"
%include "IMP/domino/SimpleDiscreteRestraint.h"
%include "IMP/domino/SimpleDiscreteSampler.h"
%include "IMP/domino/TransformationMappedDiscreteSet.h"
%include "IMP/domino/TransformationCartesianProductSampler.h"
%include "IMP/domino/TransformationPermutationSampler.h"
%include "IMP/domino/TransformationDiscreteSet.h"
%include "IMP/domino/SymmetrySampler.h"

