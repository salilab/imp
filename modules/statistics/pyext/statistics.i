%module(directors="1") "IMP.statistics"

%{
#include "IMP.h"
#include "IMP/statistics.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "statistics_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"

/* Wrap our own classes */
%include "IMP/statistics/KMLProxy.h"
%include "IMP/statistics/random_generator.h"
