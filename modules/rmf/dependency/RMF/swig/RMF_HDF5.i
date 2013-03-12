%module "RMF_HDF5"
%feature("autodoc", 1);
// turn off the warning as it mostly triggers on methods (and lots of them)
%warnfilter(321);

%{
#include <RMF/compiler_macros.h>

// needs to go beyond the POP
RMF_GCC_PRAGMA( diagnostic ignored "-Wmissing-declarations")
RMF_PUSH_WARNINGS
RMF_GCC_PRAGMA( diagnostic ignored "-Wunused-value")

/* SWIG generates long class names with wrappers that use certain Boost classes,
   longer than the 255 character name length for MSVC. This shouldn't affect
   the code, but does result in a lot of warning output, so disable this warning
   for clarity. */
RMF_VC_PRAGMA(warning( disable: 4503 ))

#include <boost/version.hpp>
#include <boost/exception/all.hpp>

#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>
#include <exception>

#include "RMF/internal/swig_helpers.h"
#include "RMF/HDF5.h"

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
%include "RMF/HDF5/infrastructure_macros.h"

%pythoncode %{
_types_list=[]
def get_data_types():
   return _types_list
%}

/* Apply the passed macro to each type used in RMF */
%define IMP_RMF_SWIG_FOREACH_HDF5_TYPE(macroname)
  macroname(int, Int, Ints, int);
macroname(ints, Ints, IntsList, RMF::HDF5::Ints);
  macroname(float, Float, Floats, double);
  macroname(floats, Floats, FloatsList, RMF::HDF5::Floats);
  macroname(index, Index, Indexes, int);
  macroname(indexes, Indexes, IndexesList, RMF::HDF5::Indexes);
  macroname(string, String, Strings, std::string);
  macroname(strings, Strings, StringsList, RMF::HDF5::Strings);
%enddef


IMP_RMF_SWIG_NATIVE_VALUES_LIST(RMF::HDF5, double, Floats, FloatsList);
IMP_RMF_SWIG_NATIVE_VALUES_LIST(RMF::HDF5, int, Ints, IntsList);
IMP_RMF_SWIG_NATIVE_VALUES_LIST(RMF::HDF5, std::string, Strings, StringsList);
IMP_RMF_SWIG_VALUE_BUILTIN(RMF::HDF5, Float, Floats, double);
IMP_RMF_SWIG_VALUE_BUILTIN(RMF::HDF5, Int, Ints, int);
IMP_RMF_SWIG_VALUE_BUILTIN(RMF::HDF5, String, Strings, std::string);
IMP_RMF_SWIG_NATIVE_VALUE(float);
IMP_RMF_SWIG_NATIVE_VALUE(double);
IMP_RMF_SWIG_NATIVE_VALUE(int);
IMP_RMF_SWIG_NATIVE_VALUE(std::string);

/* Declare the needed things for each type */
%define IMP_RMF_SWIG_DECLARE_HDF5_TYPE(lcname, Ucname, Ucnames, Type)
namespace RMF {
  namespace HDF5 {
    %rename(_##Ucname##Traits) Ucname##Traits;
  }
}
%inline %{
namespace RMF {
  namespace HDF5 {
 const Type Null##Ucname=Ucname##Traits::get_null_value();
  }
}
%}
%pythoncode %{
_types_list.append(#lcname)
%}
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, Ucname##DataSet1D, Ucname##DataSet1D, Ucname##DataSet1Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, Ucname##DataSet2D, Ucname##DataSet2D, Ucname##DataSet2Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, Ucname##DataSet3D, Ucname##DataSet3D, Ucname##DataSet3Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, Ucname##DataSet1DAttributes, Ucname##DataSet1DAttributes, Ucname##DataSet1DAttributesList);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, Ucname##DataSet2DAttributes, Ucname##DataSet2DAttributes, Ucname##DataSet2DAttributesList);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, Ucname##DataSet3DAttributes, Ucname##DataSet3DAttributes, Ucname##DataSet3DAttributesList);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, Ucname##ConstDataSet1D, Ucname##ConstDataSet1D, Ucname##ConstDataSet1Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, Ucname##ConstDataSet2D, Ucname##ConstDataSet2D, Ucname##ConstDataSet2Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, Ucname##ConstDataSet3D, Ucname##ConstDataSet3D, Ucname##ConstDataSet3Ds);
%enddef


%define IMP_RMF_SWIG_DEFINE_INTERMEDIATE_HDF5_TYPE(lcname, Ucname, Ucnames, Type)
%template(Ucname##ConstDataSet1D) RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 1>;
%template(Ucname##ConstDataSet2D) RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 2>;
%template(Ucname##ConstDataSet3D) RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 3>;
%template(Ucname##DataSetAttributes1D) RMF::HDF5::MutableAttributes< RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 1> >;
%template(Ucname##DataSetAttributes2D) RMF::HDF5::MutableAttributes< RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 2> >;
%template(Ucname##DataSetAttributes3D) RMF::HDF5::MutableAttributes< RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 3> >;
%enddef

%define IMP_RMF_SWIG_DEFINE_HDF5_TYPE(lcname, Ucname, Ucnames, Type)
%template(Ucname##DataSet1D) RMF::HDF5::DataSetD<RMF::HDF5::Ucname##Traits, 1>;
%template(Ucname##DataSet2D) RMF::HDF5::DataSetD<RMF::HDF5::Ucname##Traits, 2>;
%template(Ucname##DataSet3D) RMF::HDF5::DataSetD<RMF::HDF5::Ucname##Traits, 3>;
%enddef

IMP_RMF_SWIG_VALUE(RMF::HDF5, Object, Objects);
IMP_RMF_SWIG_VALUE_TEMPLATE(RMF::HDF5, ConstAttributes);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, ConstGroupAttributes,ConstGroupAttributes, ConstGroupAttributesList);
IMP_RMF_SWIG_VALUE(RMF::HDF5, ConstGroup, ConstGroups);
IMP_RMF_SWIG_VALUE(RMF::HDF5, ConstFile, ConstFiles);
IMP_RMF_SWIG_VALUE_TEMPLATE(RMF::HDF5, MutableAttributes);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, GroupAttributes,GroupAttributes, GroupAttributesList);
IMP_RMF_SWIG_VALUE(RMF::HDF5, Group, Groups);
IMP_RMF_SWIG_VALUE(RMF::HDF5, File, Files);
IMP_RMF_SWIG_VALUE_TEMPLATE(RMF::HDF5, DataSetD);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, DataSetIndex1D, DataSetIndex1D, DataSetIndex1Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, DataSetIndex2D, DataSetIndex2D, DataSetIndex2Ds);
IMP_RMF_SWIG_VALUE_INSTANCE(RMF::HDF5, DataSetIndex3D, DataSetIndex3D, DataSetIndex3Ds);

IMP_RMF_SWIG_FOREACH_HDF5_TYPE(IMP_RMF_SWIG_DECLARE_HDF5_TYPE);

%implicitconv;

// char is special cased since it is just used for attributes
namespace RMF {
  namespace HDF5 {
%rename(_CharTraits) CharTraits;
  }
}

%include "RMF/HDF5/types.h"
%include "RMF/HDF5/handle.h"
%include "RMF/HDF5/Object.h"
%include "RMF/HDF5/ConstAttributes.h"
%template(_ConstAttributesObject) RMF::HDF5::ConstAttributes<RMF::HDF5::Object>;
%include "RMF/HDF5/MutableAttributes.h"
%include "RMF/HDF5/DataSetIndexD.h"
%template(DataSetIndex1D) RMF::HDF5::DataSetIndexD<1>;
%template(DataSetIndex2D) RMF::HDF5::DataSetIndexD<2>;
%template(DataSetIndex3D) RMF::HDF5::DataSetIndexD<3>;

%include "RMF/HDF5/DataSetAccessPropertiesD.h"
%include "RMF/HDF5/DataSetCreationPropertiesD.h"
%include "RMF/HDF5/ConstDataSetD.h"
IMP_RMF_SWIG_FOREACH_HDF5_TYPE(IMP_RMF_SWIG_DEFINE_INTERMEDIATE_HDF5_TYPE);
%include "RMF/HDF5/DataSetD.h"


IMP_RMF_SWIG_FOREACH_HDF5_TYPE(IMP_RMF_SWIG_DEFINE_HDF5_TYPE);

%include "RMF/HDF5/ConstGroup.h"
%template(_HDF5MutableAttributesGroup) RMF::HDF5::MutableAttributes<RMF::HDF5::ConstGroup>;
%include "RMF/HDF5/ConstFile.h"
%include "RMF/HDF5/Group.h"
%include "RMF/HDF5/File.h"

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
%}

%{
RMF_POP_WARNINGS
%}

