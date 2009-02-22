%module(directors="1") "IMP.display"

%{
#include "IMP.h"
#include "IMP/display.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
%}


%include "kernel/pyext/IMP_macros.i"
%include "kernel/pyext/IMP_exceptions.i"

%include "display_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel/pyext/IMP.i"
%import "modules/core/pyext/core.i"
%import "modules/algebra/pyext/algebra.i"

namespace IMP {
namespace display {
IMP_OWN_CONSTRUCTOR(LogOptimizerState)
IMP_OWN_METHOD(LogOptimizerState, add_geometry)
}
}

/* Wrap our own classes */
%include "IMP/display/geometry.h"
%include "IMP/display/Writer.h"

%include "IMP/display/xyzr_geometry.h"
%include "IMP/display/bond_geometry.h"
%include "IMP/display/VRMLWriter.h"
%include "IMP/display/BildWriter.h"
%include "IMP/display/CMMWriter.h"
%include "IMP/display/LogOptimizerState.h"
%include "IMP/display/SphereGeometry.h"
