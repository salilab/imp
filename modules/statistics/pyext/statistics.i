%module(directors="1") "IMP.statistics"

%{
#include "IMP.h"
#include "IMP/statistics.h"
%}

%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "modules/statistics/pyext/statistics_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/kernel.i"

/* Wrap our own classes */
%include "IMP/statistics/KMLProxy.h"
