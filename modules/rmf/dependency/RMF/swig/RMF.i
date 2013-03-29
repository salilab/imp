%module "RMF"
%feature("autodoc", 1);
// turn off the warning as it mostly triggers on methods (and lots of them)
%warnfilter(321);

%{
#include <RMF/compiler_macros.h>

RMF_PUSH_WARNINGS
RMF_GCC_PRAGMA(diagnostic ignored "-Wunused-but-set-variable")
RMF_GCC_PRAGMA(diagnostic ignored "-Wunused-value")
RMF_GCC_PRAGMA(diagnostic ignored "-Wmissing-declarations")
RMF_CLANG_PRAGMA(diagnostic ignored "-Wunused-parameter")
RMF_CLANG_PRAGMA(diagnostic ignored "-Wused-but-marked-unused")
RMF_CLANG_PRAGMA(diagnostic ignored "-Wunused-function")

/* SWIG generates long class names with wrappers that use certain Boost classes,
   longer than the 255 character name length for MSVC. This shouldn't affect
   the code, but does result in a lot of warning output, so disable this warning
   for clarity. */
RMF_VC_PRAGMA( warning( disable: 4503 ) )

#include <boost/version.hpp>
#include <boost/exception/exception.hpp>

#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>
#include <exception>

#include "RMF/internal/swig_helpers.h"
#include "RMF.h"

%}
%include "std_vector.i"
%include "std_string.i"
%include "std_pair.i"

%include "RMF/config.h"

%pythoncode %{
_value_types=[]
_object_types=[]
_raii_types=[]
_plural_types=[]
%}



%include "typemaps.i"

%include "RMF.types.i"
%include "RMF.exceptions.i"
%include "RMF/compiler_macros.h"
%include "RMF/infrastructure_macros.h"

%pythoncode %{
_types_list=[]
def get_data_types():
   return _types_list
%}


/* Apply the passed macro to each type used in RMF */
%define IMP_RMF_SWIG_FOREACH_TYPE(macroname)
  macroname(int, Int, Ints, int);
  macroname(ints, Ints, IntsList, RMF::Ints);
  macroname(float, Float, Floats, double);
  macroname(floats, Floats, FloatsList, RMF::Floats);
  macroname(index, Index, Indexes, int);
  macroname(indexes, Indexes, IndexesList, RMF::Indexes);
  macroname(string, String, Strings, std::string);
  macroname(strings, Strings, StringsList, RMF::Strings);
  macroname(node_id, NodeID, NodeIDs, RMF::NodeID);
  macroname(node_ids, NodeIDs, NodeIDsList, RMF::NodeIDs);
%enddef

/* Declare the needed things for each type */
%define IMP_RMF_SWIG_DECLARE_TYPE(lcname, Ucname, Ucnames, Type)
%inline %{
namespace RMF {
 const Type Null##Ucname=Ucname##Traits::get_null_value();
}
%}
namespace RMF {
%rename(_##Ucname##Traits) Ucname##Traits;
}

IMP_RMF_SWIG_VALUE_INSTANCE(RMF, Ucname##Key, Ucname##Key, Ucname##Keys);
%pythoncode %{
_types_list.append(#lcname)
%}
%enddef

%define IMP_RMF_SWIG_DEFINE_TYPE(lcname, Ucname, Ucnames, Type)
%template(Ucname##Key) RMF::Key<RMF::Ucname##Traits>;
%enddef

IMP_RMF_SWIG_VALUE(RMF, NodeID, NodeIDs);
IMP_RMF_SWIG_VALUE(RMF, FrameID, FrameIDs);
IMP_RMF_SWIG_NATIVE_VALUES_LIST(RMF, double, Floats, FloatsList);
IMP_RMF_SWIG_NATIVE_VALUES_LIST(RMF, int, Ints, IntsList);
IMP_RMF_SWIG_NATIVE_VALUES_LIST(RMF, std::string, Strings, StringsList);
IMP_RMF_SWIG_VALUE_BUILTIN(RMF, Float, Floats, double);
IMP_RMF_SWIG_VALUE_BUILTIN(RMF, Int, Ints, int);
IMP_RMF_SWIG_VALUE_BUILTIN(RMF, String, Strings, std::string);
IMP_RMF_SWIG_NATIVE_VALUE(float);
IMP_RMF_SWIG_NATIVE_VALUE(double);
IMP_RMF_SWIG_NATIVE_VALUE(int);
IMP_RMF_SWIG_NATIVE_VALUE(std::string);


IMP_RMF_SWIG_GRAPH(RMF, NodeTree, NodeTree, RMF::NodeHandle);
IMP_RMF_SWIG_VALUE(RMF, NodeConstHandle, NodeConstHandles);
IMP_RMF_SWIG_VALUE(RMF, FrameConstHandle, FrameConstHandles);
IMP_RMF_SWIG_VALUE(RMF, FileConstHandle, FileConstHandles);
IMP_RMF_SWIG_VALUE(RMF, NodeHandle, NodeHandles);
IMP_RMF_SWIG_VALUE(RMF, FrameHandle, FrameHandles);
IMP_RMF_SWIG_VALUE(RMF, FileHandle, FileHandles);
IMP_RMF_SWIG_VALUE(RMF, SetCurrentFrame, SetCurrentFrames);
IMP_RMF_SWIG_VALUE(RMF, Category, Categories);
IMP_RMF_SWIG_VALUE_TEMPLATE(RMF, Key);
IMP_RMF_SWIG_PAIR(RMF, Index, IndexRange, IndexRanges)
IMP_RMF_SWIG_PAIR(RMF, Int, IntRange, IntRanges)


IMP_RMF_SWIG_VALUE(RMF, ReferenceFrame, ReferenceFrames);


IMP_RMF_SWIG_FOREACH_TYPE(IMP_RMF_SWIG_DECLARE_TYPE);

%implicitconv;

%extend RMF::FileConstHandle {
   %pythoncode %{
    def get_keys(self, kc):
        ret=[]
        for t in _types_list:
           fn= getattr(self, "get_"+t+"_keys")
           ret.extend(fn(kc))
        return ret
  %}
}

%include "RMF/constants.h"
%include "RMF/NodeID.h"
%include "RMF/FrameID.h"
%include "RMF/types.h"

%include "RMF/Key.h"
IMP_RMF_SWIG_FOREACH_TYPE(IMP_RMF_SWIG_DEFINE_TYPE);


%include "RMF/Category.h"

%include "RMF/names.h"

%include "RMF/NodeConstHandle.h"
%include "RMF/FrameConstHandle.h"
%include "RMF/NodeHandle.h"
%include "RMF/FrameHandle.h"
%include "RMF/FileConstHandle.h"
%include "RMF/FileHandle.h"
%include "RMF/Validator.h"
%include "RMF/Decorator.h"
namespace RMF {
  %template(_Decorator) Decorator<NodeHandle>;
  %template(_ConstDecorator) Decorator<NodeConstHandle>;
}

%include "RMF/Factory.h"
namespace RMF {
  %template(_Factory) Factory<FileHandle>;
  %template(_ConstFactory) Factory<FileConstHandle>;
}


IMP_RMF_DECORATOR(RMF, Particle);
IMP_RMF_DECORATOR(RMF, IntermediateParticle);
IMP_RMF_DECORATOR(RMF, Colored);
IMP_RMF_DECORATOR(RMF, External);
IMP_RMF_DECORATOR(RMF, JournalArticle);
IMP_RMF_DECORATOR(RMF, Ball);
IMP_RMF_DECORATOR(RMF, Cylinder);
IMP_RMF_DECORATOR(RMF, Segment);
IMP_RMF_DECORATOR(RMF, Score);
IMP_RMF_DECORATOR(RMF, RigidParticle);
IMP_RMF_DECORATOR(RMF, ReferenceFrame);
IMP_RMF_DECORATOR(RMF, Residue);
IMP_RMF_DECORATOR(RMF, Atom);
IMP_RMF_DECORATOR(RMF, Alias);
IMP_RMF_DECORATOR(RMF, Bond);
IMP_RMF_DECORATOR(RMF, Chain);
IMP_RMF_DECORATOR(RMF, Domain);
IMP_RMF_DECORATOR(RMF, Copy);
IMP_RMF_DECORATOR(RMF, Diffuser);
IMP_RMF_DECORATOR(RMF, Typed);
IMP_RMF_DECORATOR(RMF, Force);
IMP_RMF_DECORATOR(RMF, Torque);

%include "RMF/physics_decorators.h"
%include "RMF/sequence_decorators.h"
%include "RMF/shape_decorators.h"
%include "RMF/external_decorators.h"
%include "RMF/feature_decorators.h"
%include "RMF/alias_decorators.h"
%include "RMF/publication_decorators.h"
%include "RMF/decorator_utility.h"
%include "RMF/utility.h"
%include "RMF/SetCurrentFrame.h"
%include "RMF/CoordinateTransformer.h"
%include "RMF/log.h"

%pythoncode %{
_tmpdir=None

def _get_temporary_file_path(name):
   global _tmpdir
   if not _tmpdir:
       import tempfile
       _tmpdir = tempfile.mkdtemp()
   import os.path
   return os.path.join(_tmpdir, name)

def _get_test_input_file_path(name):
   import sys
   import os.path
   dir= os.path.split(sys.argv[0])[0]
   return os.path.join(dir, "input", name)

def get_example_path(name):
   import sys
   import os.path
   dir= os.path.split(sys.argv[0])[0]
   return os.path.join(dir, name)

suffixes=["rmf", "rmf2", "rmfa", "rmft"]

read_write_suffixes=["rmf", "rmfa", "rmft"]

import RMF_HDF5
HDF5=RMF_HDF5

%}

%{
RMF_POP_WARNINGS
%}
