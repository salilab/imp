%module(directors="1") "IMP.saxs"

%{
#include "IMP.h"
#include "IMP/saxs.h"
%}


%include "IMP_macros.i"
%include "IMP_exceptions.i"
%include "IMP_streams.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of base classes (but do not wrap) */
%import "kernel.i"
%import "algebra.i"

%import "IMP/saxs/config.h"

/* Ignore some stuff to quell SWIG warnings */
%ignore operator>>(std::istream& s,
                   AtomFactorCoefficients& atom_factor_coefficients);
%ignore operator<<(std::ostream& s,
                   const AtomFactorCoefficients& atom_factor_coefficients);
%ignore operator<<(std::ostream& q, const IntensityEntry& e);
%ignore operator>>(std::istream& q, IntensityEntry& e);


/* Wrap our own classes */
%include "IMP/saxs/FormFactorTable.h"
%include "IMP/saxs/Profile.h"
%include "IMP/saxs/Score.h"
%include "IMP/saxs/Restraint.h"
%include "IMP/saxs/Distribution.h"
%include "IMP/saxs/utility.h"
