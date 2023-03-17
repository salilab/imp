%module "RMF_HDF5"
%feature("autodoc", 1);

%include "RMF.warnings.i"

%{
#include <RMF/compiler_macros.h>
/* SWIG generates long class names with wrappers that use certain Boost classes,
   longer than the 255 character name length for MSVC. This shouldn't affect
   the code, but does result in a lot of warning output, so disable this warning
   for clarity. */
RMF_VC_PRAGMA(warning( disable: 4503 ))

#include <boost/version.hpp>
#include <boost/exception/all.hpp>

#include <type_traits>
#include <exception>

#include "RMF/internal/swig_helpers.h"
#include "RMF/HDF5.h"

%}
%include "std_vector.i"
%include "std_string.i"
%include "std_pair.i"

%include "RMF/config.h"



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

%template(Ints) std::vector<int>;
%template(Floats) std::vector<float>;
%template(Strings) std::vector<std::string>;
%template(Doubles) std::vector<double>;
%template(IntsList) std::vector<std::vector<int> >;
%template(FloatsList) std::vector<std::vector<float> >;
%template(StringsList) std::vector<std::vector<std::string> >;
%template(DoublesList) std::vector<std::vector<double> >;


/* Apply the passed macro to each type used in RMF */
%define RMF_SWIG_FOREACH_HDF5_TYPE(macroname)
  macroname(int, Int, Ints, int);
  macroname(ints, Ints, IntsList, RMF::HDF5::Ints);
  macroname(float, Float, Floats, double);
  macroname(floats, Floats, FloatsList, RMF::HDF5::Floats);
  macroname(index, Index, Indexes, int);
  macroname(indexes, Indexes, IndexesList, RMF::HDF5::Indexes);
  macroname(string, String, Strings, std::string);
  macroname(strings, Strings, StringsList, RMF::HDF5::Strings);
%enddef


/* Declare the needed things for each type */
%define RMF_SWIG_DECLARE_HDF5_TYPE(lcname, Ucname, Ucnames, Type)
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
%enddef


%define RMF_SWIG_DEFINE_INTERMEDIATE_HDF5_TYPE(lcname, Ucname, Ucnames, Type)
%template(Ucname##ConstDataSet1D) RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 1>;
%template(Ucname##ConstDataSet2D) RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 2>;
%template(Ucname##ConstDataSet3D) RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 3>;
%template(Ucname##DataSetAttributes1D) RMF::HDF5::MutableAttributes< RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 1> >;
%template(Ucname##DataSetAttributes2D) RMF::HDF5::MutableAttributes< RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 2> >;
%template(Ucname##DataSetAttributes3D) RMF::HDF5::MutableAttributes< RMF::HDF5::ConstDataSetD<RMF::HDF5::Ucname##Traits, 3> >;
%enddef

%define RMF_SWIG_DEFINE_HDF5_TYPE(lcname, Ucname, Ucnames, Type)
%template(Ucname##DataSet1D) RMF::HDF5::DataSetD<RMF::HDF5::Ucname##Traits, 1>;
%template(Ucname##DataSet2D) RMF::HDF5::DataSetD<RMF::HDF5::Ucname##Traits, 2>;
%template(Ucname##DataSet3D) RMF::HDF5::DataSetD<RMF::HDF5::Ucname##Traits, 3>;
%enddef


RMF_SWIG_FOREACH_HDF5_TYPE(RMF_SWIG_DECLARE_HDF5_TYPE);

%implicitconv;

// char is special cased since it is just used for attributes
namespace RMF {
  namespace HDF5 {
%rename(_CharTraits) CharTraits;
  }
}

%include "RMF/HDF5/types.h"

%template(_IntTraitsBaseClass) RMF::HDF5::SimpleTraits<RMF::HDF5::IntTraitsBase>;
%template(_IntsTraitsBase) RMF::HDF5::SimplePluralTraits<RMF::HDF5::IntTraits>;
%template(_FloatTraitsBaseClass) RMF::HDF5::SimpleTraits<RMF::HDF5::FloatTraitsBase>;
%template(_FloatsTraitsBase) RMF::HDF5::SimplePluralTraits<RMF::HDF5::FloatTraits>;
%template(_IndexTraitsBaseClass) RMF::HDF5::SimpleTraits<RMF::HDF5::IndexTraitsBase>;
%template(_IndexesTraitsBase) RMF::HDF5::SimplePluralTraits<RMF::HDF5::IndexTraits>;
%rename(_IntTraitsBase) IntTraitsBase;
%rename(_FloatTraitsBase) FloatTraitsBase;
%rename(_IndexTraitsBase) IndexTraitsBase;

%inline %{
  namespace RMF {
    namespace HDF5 {
#ifdef SWIG
struct IntTraits : public RMF::HDF5::SimpleTraits<RMF::HDF5::IntTraitsBase> {};
struct IntsTraits : public SimplePluralTraits<IntTraits> {};
struct FloatTraits : public SimpleTraits<FloatTraitsBase> {};
struct FloatsTraits : public SimplePluralTraits<FloatTraits> {};
struct IndexTraits : public SimpleTraits<IndexTraitsBase> {};
struct IndexesTraits : public SimplePluralTraits<IndexTraits> {};
#endif
    }
  }
%}

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
RMF_SWIG_FOREACH_HDF5_TYPE(RMF_SWIG_DEFINE_INTERMEDIATE_HDF5_TYPE);
%include "RMF/HDF5/DataSetD.h"


RMF_SWIG_FOREACH_HDF5_TYPE(RMF_SWIG_DEFINE_HDF5_TYPE);

%include "RMF/HDF5/ConstGroup.h"
%template(_HDF5MutableAttributesGroup) RMF::HDF5::MutableAttributes<RMF::HDF5::ConstGroup>;
%include "RMF/HDF5/ConstFile.h"
%include "RMF/HDF5/Group.h"
%include "RMF/HDF5/File.h"
