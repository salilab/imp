%module(directors="1") "IMP.saxs"

%{
#include "IMP.h"
#include "IMP/saxs.h"
%}


%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "saxs_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "modules/algebra/pyext/algebra.i"

namespace IMP::saxs {
  IMP_OWN_FIRST_CONSTRUCTOR(SaxsData)
}

/* Wrap our own classes */
%include "IMP/saxs/FormFactorTable.h"
%include "IMP/saxs/RadialDistributionFunction.h"
%include "IMP/saxs/SAXSProfile.h"
%include "IMP/saxs/SAXSScore.h"
%include "IMP/saxs/SaxsData.h"
