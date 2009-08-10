%module(directors="1") "IMP.domino"

%{
#include "IMP.h"
#include "IMP/domino.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
%}

%include "IMP_macros.i"
%include "IMP_exceptions.i"
%include "IMP_streams.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Make selected classes extensible in Python */
%import "IMP_directors.i"
IMP_DIRECTOR_MODULE_CLASS(domino, DiscreteSampler);
IMP_DIRECTOR_MODULE_CLASS(domino, DiscreteSet);

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel.i"
%import "algebra.i"

%include "IMP/domino/config.h"

/* Wrap our own classes */
%include "IMP/domino/JunctionTree.h"
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

