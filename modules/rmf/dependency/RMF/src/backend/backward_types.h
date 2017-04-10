/**
 *  \file RMF/types.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_BACKWARD_TYPES_H
#define RMF_BACKWARD_TYPES_H

#include "RMF/config.h"
#include "RMF/types.h"

RMF_ENABLE_WARNINGS

namespace RMF {

namespace backward_types {

#define RMF_FOREACH_BACKWARDS_TYPE(macroname)                               \
  RMF_FOREACH_SIMPLE_TYPE(macroname);                                       \
  macroname(string, String, RMF::String, RMF::String, const RMF::Strings&,  \
            RMF::Strings);                                                  \
  macroname(strings, Strings, const RMF::Strings&, RMF::Strings,            \
            const RMF::StringsList&, RMF::StringsList);                     \
  macroname(node_id, NodeID, RMF::NodeID, RMF::NodeID, const RMF::NodeIDs&, \
            RMF::NodeIDs);                                                  \
  macroname(node_ids, NodeIDs, const RMF::NodeIDs&, RMF::NodeIDs,           \
            const RMF::NodeIDsList&, RMF::NodeIDsList);                     \
  macroname(floats, Floats, const RMF::Floats&, RMF::Floats,                \
            const RMF::FloatsList&, RMF::FloatsList);                       \
  macroname(ints, Ints, const RMF::Ints&, RMF::Ints, const RMF::IntsList&,  \
            RMF::IntsList);                                                 \
  macroname(indexes, Indexes, const RMF::Indexes&, RMF::Indexes,            \
            const RMF::IndexesList&, RMF::IndexesList);

struct IndexTraits {
  typedef int Type;
  typedef std::vector<int> Types;
  typedef Type ReturnType;
  typedef Type ArgumentType;
  static bool get_is_null_value(const Type& t) { return t == -1; }
  static ReturnType get_null_value() { return -1; }
  typedef HDF5::IndexTraits HDF5Traits;
  typedef boost::int32_t AvroType;
  static bool get_are_equal(ArgumentType a, ArgumentType b) { return a == b; }
  static std::string get_tag() { return "kx"; }
};

struct IndexesTraits {
  typedef IndexTraits::Types Type;
  typedef std::vector<Type> Types;
  typedef Type ReturnType;
  typedef const Type& ArgumentType;
  static bool get_is_null_value(const Type& t) { return t.empty(); }
  static const Type& get_null_value() {
    static Type r;
    return r;
  }
  typedef HDF5::IndexesTraits HDF5Traits;
  typedef std::vector<IndexTraits::AvroType> AvroType;
  static bool get_are_equal(ArgumentType a, ArgumentType b) {
    if (a.size() != b.size()) return false;
    for (unsigned int i = 0; i < a.size(); ++i) {
      if (!IndexTraits::get_are_equal(a[i], b[i])) return false;
    }
    return true;
  }
  static std::string get_tag() { return "kxs"; }
};

struct NodeIDTraits {
  typedef NodeID Type;
  typedef NodeIDs Types;
  typedef Type ReturnType;
  typedef Type ArgumentType;
  static bool get_is_null_value(const Type& t) { return t == Type(); }
  static ReturnType get_null_value() { return NodeID(); }
#ifndef SWIG
  struct HDF5Traits : public HDF5::IndexTraits {
    static int get_index() { return 4; }
    static std::string get_name() { return "node_id"; }
  };
#endif
  typedef boost::int32_t AvroType;
  static bool get_are_equal(ArgumentType a, ArgumentType b) { return a == b; }
  static std::string get_tag() { return "kn"; }
};

struct NodeIDsTraits {
  typedef NodeIDs Type;
  typedef std::vector<NodeIDs> Types;
  typedef Type ReturnType;
  typedef const Type& ArgumentType;
  static bool get_is_null_value(const Type& t) { return t.empty(); }
  static ReturnType get_null_value() {
    static Type r;
    return r;
  }
#ifndef SWIG
  struct HDF5Traits : public HDF5::IndexesTraits {
    static int get_index() { return 5; }
    static std::string get_name() { return "node_ids"; }
  };
#endif
  typedef std::vector<IndexTraits::AvroType> AvroType;
  static bool get_are_equal(ArgumentType a, ArgumentType b) {
    if (a.size() != b.size()) return false;
    for (unsigned int i = 0; i < a.size(); ++i) {
      if (!NodeIDTraits::get_are_equal(a[i], b[i])) return false;
    }
    return true;
  }
  static std::string get_tag() { return "kns"; }
};

typedef ID<IndexTraits> IndexKey;
typedef ID<IndexesTraits> IndexesKey;
typedef ID<NodeIDTraits> NodeIDKey;
typedef ID<NodeIDsTraits> NodeIDsKey;
typedef std::vector<IndexKey> IndexKeys;
typedef std::vector<IndexesKey> IndexesKeys;
typedef std::vector<NodeIDKey> NodeIDKeys;
typedef std::vector<NodeIDsKey> NodeIDsKeys;

} /* namespace backends */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_BACKWARD_TYPES_H */
