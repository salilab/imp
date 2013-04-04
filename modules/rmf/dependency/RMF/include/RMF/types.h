/**
 *  \file RMF/types.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_TYPES_H
#define RMF_TYPES_H

#include <RMF/config.h>
#include "NodeID.h"
#include "infrastructure_macros.h"
#include "internal/errors.h"
#include <RMF/HDF5/types.h>
#include <algorithm>
#include <boost/cstdint.hpp>
#include <limits>


RMF_ENABLE_WARNINGS
namespace RMF {

/** The type used to store integral values.*/
typedef int Int;
/** The type used to store lists of integral values.*/
typedef std::vector<Int> Ints;
/** The type used to store lists of floating point values.*/
typedef double Float;
/** The type used to store lists of floating point values.*/
typedef std::vector<Float> Floats;
/** The type used to store lists of floating point values.*/
typedef std::vector<Floats> FloatsList;
/** The type used to store lists of index values.*/
typedef int Index;
/** The type used to store lists of index values.*/
typedef std::vector<Index> Indexes;
/** The type used to store lists of indexes.*/
typedef std::vector<Indexes> IndexesList;
/** The type used to store lists of string values.*/
typedef std::string String;
/** The type used to store lists of string values.*/
typedef std::vector<String> Strings;
/** The type used to store lists of strings values.*/
typedef std::vector<Strings> StringsList;
/** The type used to store lists of NodeID values.*/
typedef std::vector<NodeIDs> NodeIDsList;
/** The type used to store lists of lists of integers values.*/
typedef std::vector<Ints> IntsList;
/** The type used to store char values.*/
typedef char Char;
/** The type used to store lists of char values.*/
typedef std::string Chars;

typedef std::pair<Index, Index> IndexRange;
typedef std::pair<Int, Int> IntRange;

struct IntTraits {
  typedef Int Type;
  typedef Ints Types;
  static bool get_is_null_value(const Type &t) {
    return t == get_null_value();
  }
  static Type get_null_value() {
    return std::numeric_limits<Type>::max();
  }
  typedef HDF5::IntTraits HDF5Traits;
  typedef boost::int32_t AvroType;
};
struct FloatTraits {
  typedef Float Type;
  typedef Floats Types;
  static bool get_is_null_value(const Type &t) {
    return t >= std::numeric_limits<Float>::max();
  }
  static Type get_null_value() {
    return std::numeric_limits<Type>::infinity();
  }
  typedef HDF5::FloatTraits HDF5Traits;
  typedef Type AvroType;
};
struct StringTraits {
  typedef String Type;
  typedef Strings Types;
  static bool get_is_null_value(const Type &t) {
    return t.empty();
  }
  static Type get_null_value() {
    return Type();
  }
  typedef HDF5::StringTraits HDF5Traits;
  typedef Type AvroType;
};
struct IndexTraits {
  typedef Index Type;
  typedef Indexes Types;
  static bool get_is_null_value(const Type &t) {
    return t==-1;
  }
  static Type get_null_value() {
    return -1;
  }
  typedef HDF5::IndexTraits HDF5Traits;
  typedef boost::int32_t AvroType;
};
struct IntsTraits {
  typedef Ints Type;
  typedef IntsList Types;
  static bool get_is_null_value(const Type &t) {
    return t.empty();
  }
  static Type get_null_value() {
    return Type();
  }
  typedef HDF5::IntsTraits HDF5Traits;
  typedef std::vector<IntTraits::AvroType> AvroType;
};
struct FloatsTraits {
  typedef Floats Type;
  typedef FloatsList Types;
  static bool get_is_null_value(const Type &t) {
    return t.empty();
  }
  static Type get_null_value() {
    return Type();
  }
  typedef HDF5::FloatsTraits HDF5Traits;
  typedef Type AvroType;
};
struct StringsTraits {
  typedef Strings Type;
  typedef StringsList Types;
  static bool get_is_null_value(const Type &t) {
    return t.empty();
  }
  static Type get_null_value() {
    return Type();
  }
  typedef HDF5::StringsTraits HDF5Traits;
  typedef Type AvroType;
};

struct IndexesTraits {
  typedef Indexes Type;
  typedef IndexesList Types;
  static bool get_is_null_value(const Type &t) {
    return t.empty();
  }
  static Type get_null_value() {
    return Type();
  }
  typedef HDF5::IndexesTraits HDF5Traits;
  typedef std::vector<IndexTraits::AvroType> AvroType;
};

struct NodeIDTraits {
  typedef NodeID Type;
  typedef NodeIDs Types;
  static bool get_is_null_value(const Type &t) {
    return t==Type();
  }
  static NodeID get_null_value() {return NodeID();}
#ifndef SWIG
  struct HDF5Traits: public HDF5::IndexTraits {
    static int get_index() {
      return 4;
    }
    static std::string get_name() {
      return "node_id";
    }
  };
#endif
  typedef boost::int32_t AvroType;
};

struct NodeIDsTraits {
  typedef NodeIDs Type;
  typedef NodeIDsList Types;
  static bool get_is_null_value(const Type &t) {
    return t.empty();
  }
  static NodeIDs get_null_value() {return NodeIDs();}
#ifndef SWIG
  struct HDF5Traits: public HDF5::IndexesTraits {
    static int get_index() {
      return 5;
    }
    static std::string get_name() {
      return "node_ids";
    }
  };
#endif
  typedef std::vector<IndexTraits::AvroType> AvroType;
};

/** Get one type as another, handling vectors or scalars.*/
template <class OutType, class InType>
OutType get_as(InType in) {
  return OutType(in);
}
/** NodeIDs require translation.*/
template <class Out>
Out get_as(NodeID ni) {
  return Out(ni.get_index());
}
/** Get one type as another, handling vectors or scalars.*/
template <class OutType, class InType>
OutType get_as(const std::vector<InType> in) {
  OutType ret(in.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= get_as<typename OutType::value_type>(in[i]);
  }
  return ret;
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_TYPES_H */
