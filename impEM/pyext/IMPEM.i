%module(directors="1") IMPEM

%{
#include "IMP.h"
#include "../src/IMPParticlesAccessPoint.h"
#include "../src/EMFitRestraint.h"
%}

%include "kernel/pyext/IMP_macros.i"

/* Ignore shared object import/export stuff */
#define EMDLLEXPORT
#define EMDLLLOCAL
#define IMPEMDLLEXPORT
#define IMPEMDLLLOCAL

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
%import "IMP/Object.h"
%import "IMP/VersionInfo.h"
%import "IMP/Restraint.h"

/* Get definitions of EMLIB base classes (but do not wrap) */
%import "ParticlesAccessPoint.h"

/* Wrap our own classes */
%include "../src/IMPParticlesAccessPoint.h"
%include "../src/EMFitRestraint.h"

/* Allow runtime casting of Restraint* objects to EMFitRestraint* */
namespace IMP {
  %extend EMFitRestraint {
    static EMFitRestraint* cast(Restraint *r) {
      return dynamic_cast<IMP::EMFitRestraint *>(r);
    }
  }
}
