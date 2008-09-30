%module(directors="1") IMP_em

%{
#include "IMP.h"
#include "IMP/em/IMPParticlesAccessPoint.h"
#include "IMP/em/EMFitRestraint.h"
%}

%include "kernel/pyext/IMP_macros.i"

/* Ignore shared object import/export stuff */
#define EMDLLEXPORT
#define EMDLLLOCAL
%include "em_exports.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Make selected classes extensible in Python */
%feature("director") IMP::em::IMPParticlesAccessPoint;

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"

/* Get definitions of EMBED base classes (but do not wrap) */
%import "ParticlesAccessPoint.h"

/* Don't use the exception handlers defined in the kernel */
%exception;

/* Wrap our own classes */
%include "IMP/em/IMPParticlesAccessPoint.h"
%include "IMP/em/EMFitRestraint.h"

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
