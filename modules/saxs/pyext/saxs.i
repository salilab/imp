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

/* Ignore some stuff to quell SWIG warnings */
%ignore operator>>(std::istream& s,
                   AtomFactorCoefficients& atom_factor_coefficients);
%ignore operator<<(std::ostream& s,
                   const AtomFactorCoefficients& atom_factor_coefficients);
%ignore operator<<(std::ostream& q, const IntensityEntry& e);
%ignore operator>>(std::istream& q, IntensityEntry& e);

/* Wrap our own classes */
%include "IMP/saxs/FormFactorTable.h"
%include "IMP/saxs/SAXSProfile.h"
%include "IMP/saxs/SAXSScore.h"
%include "IMP/saxs/Distribution.h"
%include "IMP/saxs/utility.h"
