%module(directors="1") "IMP.display"

%{
#include "IMP.h"
#include "IMP/display.h"
#include "IMP/algebra.h"
#include "IMP/core.h"
#include "IMP/atom.h"
%}


%include "IMP_macros.i"
%include "IMP_exceptions.i"
%include "IMP_streams.i"

%include "modules/display/pyext/display_config.i"

%include "std_vector.i"
%include "std_string.i"
%include "std_except.i"

/* Get definitions of kernel base classes (but do not wrap) */
%import "kernel.i"
%import "core.i"
%import "atom.i"
%import "algebra.i"

/* Make selected classes extensible in Python */
%import "IMP_directors.i"
IMP_DIRECTOR_MODULE_CLASS(display, Writer);
IMP_DIRECTOR_MODULE_CLASS(display, Geometry);
IMP_DIRECTOR_MODULE_CLASS(display, CompoundGeometry);

/* Wrap our own classes */
%include "IMP/display/macros.h"
%include "IMP/display/Color.h"
%include "IMP/display/geometry.h"
%include "IMP/display/Writer.h"

%include "IMP/display/xyzr_geometry.h"
%include "IMP/display/bond_geometry.h"
%include "IMP/display/derivative_geometry.h"
%include "IMP/display/VRMLWriter.h"
%include "IMP/display/BildWriter.h"
%include "IMP/display/CMMWriter.h"
%include "IMP/display/Colored.h"
%include "IMP/display/CGOWriter.h"
%include "IMP/display/ChimeraWriter.h"
%include "IMP/display/LogOptimizerState.h"
%include "IMP/display/SphereGeometry.h"
%include "IMP/display/CylinderGeometry.h"
%include "IMP/display/BoxGeometry.h"
%include "IMP/display/PolygonGeometry.h"
%include "IMP/display/TriangleGeometry.h"
%include "IMP/display/PDBOptimizerState.h"

namespace IMP {
namespace display {
  %template(Geometries) ::IMP::VectorOfRefCounted< ::IMP::display::Geometry*>;
  %template(CompoundGeometries) ::IMP::VectorOfRefCounted< ::IMP::display::CompoundGeometry*>;
 }
}
