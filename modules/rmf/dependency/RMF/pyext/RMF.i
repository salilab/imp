%module(directors="1") "RMF"
%feature("autodoc", 1);
// turn off the warning as it mostly triggers on methods (and lots of them)
%warnfilter(321);

%{
/* SWIG generates long class names with wrappers that use certain Boost classes,
   longer than the 255 character name length for MSVC. This shouldn't affect
   the code, but does result in a lot of warning output, so disable this warning
   for clarity. */
#ifdef _MSC_VER
#pragma warning( disable: 4503 )
#endif

#include <boost/version.hpp>
#include <boost/exception/all.hpp>

#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>
#include <exception>


#include "RMF/config.h"
#include "RMF/constants.h"
#include "RMF/NodeID.h"
#include "RMF/types.h"
#include "RMF/hdf5_handle.h"
#include "RMF/HDF5Object.h"
#include "RMF/HDF5ConstAttributes.h"
#include "RMF/HDF5MutableAttributes.h"
#include "RMF/HDF5DataSetIndexD.h"
#include "RMF/HDF5DataSetAccessPropertiesD.h"
#include "RMF/HDF5DataSetCreationPropertiesD.h"
#include "RMF/HDF5ConstDataSetD.h"
#include "RMF/Key.h"
#include "RMF/HDF5ConstGroup.h"
#include "RMF/HDF5ConstFile.h"
#include "RMF/HDF5Group.h"
#include "RMF/HDF5File.h"
#include "RMF/Category.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/NodeHandle.h"
#include "RMF/FileConstHandle.h"
#include "RMF/FileHandle.h"
#include "RMF/names.h"
#include "RMF/Validator.h"
#include "RMF/Decorator.h"
#include "RMF/Factory.h"
#include "RMF/decorators.h"
#include "RMF/decorator_utility.h"
#include "RMF/CoordinateTransformer.h"
#include "RMF/utility.h"
#include <RMF/internal/swig_helpers.h>
#include <RMF/SetCurrentFrame.h>
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
%include "RMF/infrastructure_macros.h"

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

%pythoncode %{
_types_list=[]
def get_data_types():
   return _types_list
%}




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
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5##Ucname##DataSet1D, HDF5##Ucname##DataSet1D, HDF5##Ucname##DataSet1Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5##Ucname##DataSet2D, HDF5##Ucname##DataSet2D, HDF5##Ucname##DataSet2Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5##Ucname##DataSet3D, HDF5##Ucname##DataSet3D, HDF5##Ucname##DataSet3Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5##Ucname##DataSet1DAttributes, HDF5##Ucname##DataSet1DAttributes, HDF5##Ucname##DataSet1DAttributesList);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5##Ucname##DataSet2DAttributes, HDF5##Ucname##DataSet2DAttributes, HDF5##Ucname##DataSet2DAttributesList);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5##Ucname##DataSet3DAttributes, HDF5##Ucname##DataSet3DAttributes, HDF5##Ucname##DataSet3DAttributesList);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5##Ucname##ConstDataSet1D, HDF5##Ucname##ConstDataSet1D, HDF5##Ucname##ConstDataSet1Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5##Ucname##ConstDataSet2D, HDF5##Ucname##ConstDataSet2D, HDF5##Ucname##ConstDataSet2Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5##Ucname##ConstDataSet3D, HDF5##Ucname##ConstDataSet3D, HDF5##Ucname##ConstDataSet3Ds);
%pythoncode %{
_types_list.append(#lcname)
%}
%enddef

%define IMP_RMF_SWIG_DEFINE_INTERMEDIATE_TYPE(lcname, Ucname, Ucnames, Type)
%template(HDF5##Ucname##ConstDataSet1D) RMF::HDF5ConstDataSetD<RMF::Ucname##Traits, 1>;
%template(HDF5##Ucname##ConstDataSet2D) RMF::HDF5ConstDataSetD<RMF::Ucname##Traits, 2>;
%template(HDF5##Ucname##ConstDataSet3D) RMF::HDF5ConstDataSetD<RMF::Ucname##Traits, 3>;
%template(HDF5##Ucname##DataSetAttributes1D) RMF::HDF5MutableAttributes< RMF::HDF5ConstDataSetD<RMF::Ucname##Traits, 1> >;
%template(HDF5##Ucname##DataSetAttributes2D) RMF::HDF5MutableAttributes< RMF::HDF5ConstDataSetD<RMF::Ucname##Traits, 2> >;
%template(HDF5##Ucname##DataSetAttributes3D) RMF::HDF5MutableAttributes< RMF::HDF5ConstDataSetD<RMF::Ucname##Traits, 3> >;
%enddef

%define IMP_RMF_SWIG_DEFINE_TYPE(lcname, Ucname, Ucnames, Type)
%template(HDF5##Ucname##DataSet1D) RMF::HDF5DataSetD<RMF::Ucname##Traits, 1>;
%template(HDF5##Ucname##DataSet2D) RMF::HDF5DataSetD<RMF::Ucname##Traits, 2>;
%template(HDF5##Ucname##DataSet3D) RMF::HDF5DataSetD<RMF::Ucname##Traits, 3>;
%template(Ucname##Key) RMF::Key<RMF::Ucname##Traits>;
%enddef

IMP_RMF_SWIG_VALUE(RMF, NodeID, NodeIDs);

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
IMP_RMF_SWIG_VALUE(RMF, FileConstHandle, FileConstHandles);
IMP_RMF_SWIG_VALUE(RMF, NodeHandle, NodeHandles);
IMP_RMF_SWIG_VALUE(RMF, FileHandle, FileHandles);
IMP_RMF_SWIG_VALUE(RMF, SetCurrentFrame, SetCurrentFrames);
IMP_RMF_SWIG_VALUE(RMF, Category, Categories);
IMP_RMF_SWIG_VALUE_TEMPLATE(RMF, Key);
IMP_RMF_SWIG_VALUE(RMF, HDF5Object, HDF5Objects);
IMP_RMF_SWIG_VALUE_TEMPLATE(RMF, HDF5ConstAttributes);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5ConstGroupAttributes,HDF5ConstGroupAttributes, HDF5ConstGroupAttributesList);
IMP_RMF_SWIG_VALUE(RMF, HDF5ConstGroup, HDF5ConstGroups);
IMP_RMF_SWIG_VALUE(RMF, HDF5ConstFile, HDF5ConstFiles);
IMP_RMF_SWIG_VALUE_TEMPLATE(RMF, HDF5MutableAttributes);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5GroupAttributes,HDF5GroupAttributes, HDF5GroupAttributesList);
IMP_RMF_SWIG_VALUE(RMF, HDF5Group, HDF5Groups);
IMP_RMF_SWIG_VALUE(RMF, HDF5File, HDF5Files);
IMP_RMF_SWIG_VALUE_TEMPLATE(RMF, HDF5DataSetD);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5DataSetIndex1D, HDF5DataSetIndex1D, HDF5DataSetIndex1Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5DataSetIndex2D, HDF5DataSetIndex2D, HDF5DataSetIndex2Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF, HDF5DataSetIndex3D, HDF5DataSetIndex3D, HDF5DataSetIndex3Ds);
IMP_RMF_SWIG_PAIR(RMF, NodeConstHandle, BondPair, BondPairs)
IMP_RMF_SWIG_PAIR(RMF, Index, IndexRange, IndexRanges)
IMP_RMF_SWIG_PAIR(RMF, Int, IntRange, IntRanges)

IMP_RMF_SWIG_VALUE(RMF, ReferenceFrame, ReferenceFrames);


IMP_RMF_SWIG_FOREACH_TYPE(IMP_RMF_SWIG_DECLARE_TYPE);

%implicitconv;
%implicitconv RMF::HDF5File;
%implicitconv RMF::HDF5ConstFile;


// char is special cased since it is just used for attributes
namespace RMF {
%rename(_CharTraits) CharTraits;
}

%extend RMF::FileConstHandle {
   %pythoncode %{
    def get_keys(self, kc, arity=1):
        ret=[]
        for t in _types_list:
           fn= getattr(self, "get_"+t+"_keys")
           ret.extend(fn(kc))
        return ret
  %}
  %pythoncode %{
    def get_set_categories(self, arity):
        ret=[]
        if arity==1:
          return self.get_categories()
        if arity==2:
          return self.get_pair_categories()
        elif arity==3:
          return self.get_triplet_categories()
        elif arity==4:
          return self.get_quad_categories()
        else:
          return []
  %}
}

%extend RMF::FileHandle {
  %pythoncode %{
    def add_category(self, name, arity=1):
        ret=[]
        def call_it(self, *args):
          return _RMF.FileHandle_add_category(self, *args)
        if arity==1:
          return call_it(self, name)
        if arity==2:
          return self.add_pair_category(name)
        elif arity==3:
          return self.add_triplet_category(name)
        elif arity==4:
          return self.add_quad_category(name)
        else:
          return []
  %}
}


%include "RMF/constants.h"
%include "RMF/NodeID.h"
%include "RMF/types.h"
%include "RMF/hdf5_handle.h"
%include "RMF/HDF5Object.h"
%include "RMF/HDF5ConstAttributes.h"
%template(_HDF5ConstAttributesObject) RMF::HDF5ConstAttributes<RMF::HDF5Object>;
%include "RMF/HDF5MutableAttributes.h"
%include "RMF/HDF5DataSetIndexD.h"
%template(HDF5DataSetIndex1D) RMF::HDF5DataSetIndexD<1>;
%template(HDF5DataSetIndex2D) RMF::HDF5DataSetIndexD<2>;
%template(HDF5DataSetIndex3D) RMF::HDF5DataSetIndexD<3>;

%include "RMF/HDF5DataSetAccessPropertiesD.h"
%include "RMF/HDF5DataSetCreationPropertiesD.h"
%include "RMF/HDF5ConstDataSetD.h"
IMP_RMF_SWIG_FOREACH_TYPE(IMP_RMF_SWIG_DEFINE_INTERMEDIATE_TYPE);
%include "RMF/HDF5DataSetD.h"


%include "RMF/Key.h"
IMP_RMF_SWIG_FOREACH_TYPE(IMP_RMF_SWIG_DEFINE_TYPE);


%include "RMF/Category.h"

%include "RMF/names.h"

%include "RMF/HDF5ConstGroup.h"
%template(_HDF5MutableAttributesGroup) RMF::HDF5MutableAttributes<RMF::HDF5ConstGroup>;
%include "RMF/HDF5ConstFile.h"
%include "RMF/HDF5Group.h"
%include "RMF/HDF5File.h"

%include "RMF/NodeConstHandle.h"
%include "RMF/NodeHandle.h"
%include "RMF/FileConstHandle.h"
%include "RMF/FileHandle.h"
%include "RMF/Validator.h"
%include "RMF/Decorator.h"
%include "RMF/Factory.h"



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
IMP_RMF_DECORATOR(RMF, Chain);
IMP_RMF_DECORATOR(RMF, Domain);
IMP_RMF_DECORATOR(RMF, Copy);
IMP_RMF_DECORATOR(RMF, Diffuser);
IMP_RMF_DECORATOR(RMF, Typed);

%include "RMF/decorators.h"
%include "RMF/decorator_utility.h"
%include "RMF/utility.h"
%include "RMF/SetCurrentFrame.h"
%include "RMF/CoordinateTransformer.h"


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

suffixes=["rmf", "rmf2", "rmfa"]

read_write_suffixes=["rmf", "rmfa"]

%}
