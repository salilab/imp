%module(directors="1") "IMP.statistics"

%{
#include "IMP.h"
#include "IMP/statistics.h"
%}

%include "IMP_macros.i"
%include "IMP_exceptions.i"

%include "IMP/statistics/config.h"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel.i"

/* Wrap our own classes */
%include "IMP/statistics/KMLProxy.h"
