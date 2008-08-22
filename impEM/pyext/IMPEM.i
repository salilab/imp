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

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"

/* Get definitions of EMBED base classes (but do not wrap) */
%import "ParticlesAccessPoint.h"

/* Don't use the exception handlers defined in the kernel */
%exception;

/* Wrap our own classes */
%include "../src/IMPParticlesAccessPoint.h"
%include "../src/EMFitRestraint.h"

/* Allow runtime casting of Restraint* objects to EMFitRestraint* */
namespace IMP {
  namespace em {
    %extend EMFitRestraint {
      static EMFitRestraint* cast(Restraint *r) {
        return dynamic_cast<IMP::em::EMFitRestraint *>(r);
      }
    }
  }
}
