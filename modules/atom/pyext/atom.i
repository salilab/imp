%module(directors="1") "IMP.atom"

%{
#include "IMP.h"
#include "IMP/core.h"
#include "IMP/algebra.h"
#include "IMP/atom.h"
%}


%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "atom_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "modules/core/pyext/core.i"
%import "modules/algebra/pyext/algebra.i"

namespace IMP {
namespace atom {
}
}

/* Wrap our own classes */
%include "IMP/atom/estimates.h"
%include "IMP/atom/selectors.h"
%include "IMP/atom/PDBReader.h"
%include "IMP/atom/PDBParser.h"
