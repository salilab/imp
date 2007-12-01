%module(directors="1") IMPEM
%{
#include "IMP.h"
#include "../IMPParticlesAccessPoint.h"
#include "../EMFitRestraint.h"
%}

%include "pyext/IMP_macros.i"

/* Ignore shared object import/export stuff */
#define EMDLLEXPORT
#define EMDLLLOCAL

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

%feature("director");

namespace IMP {
  class Model;
  typedef float Float;
  class FloatKey;
  class DerivativeAccumulator;
  class Particle;
}

/* Get definitions of IMP base classes (but do not wrap; that is done by IMP) */
%import "IMP/restraints/Restraint.h"

/* Get definitions of EMLIB base classes (but do not wrap) */
%import "ParticlesAccessPoint.h"

/* Wrap our own classes */
%include "../IMPParticlesAccessPoint.h"
%include "../EMFitRestraint.h"
