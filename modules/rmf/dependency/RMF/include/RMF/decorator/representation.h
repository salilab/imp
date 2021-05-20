/**
 *  \file RMF/decorator/representation.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_DECORATOR_REPRESENTATION_H
#define RMF_DECORATOR_REPRESENTATION_H

#include <RMF/config.h>
#include <RMF/infrastructure_macros.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/Decorator.h>
#include <RMF/constants.h>
#include <RMF/Vector.h>
#include <RMF/internal/paths.h>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>

RMF_ENABLE_WARNINGS
namespace RMF {
namespace decorator {

/** See also Representation and RepresentationFactory.
  */
class RepresentationConst : public Decorator {
  friend class RepresentationFactory;
  friend class Representation;
  IntsKey representation_;
  template <class T>
  std::vector<T> get_it(const Ints &in) const {
    std::vector<T> ret;
    RMF_FOREACH(int i, in) {
      ret.push_back(get_node().get_file().get_node(NodeID(i)));
    }
    return ret;
  }

  RepresentationConst(NodeConstHandle nh, IntsKey representation)
      : Decorator(nh), representation_(representation) {}

 public:
  NodeConstHandles get_representation() const {
    try {
      return get_it<NodeConstHandle>(get_node().get_value(representation_));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeConstHandles get_frame_representation() const {
    try {
      return get_it<NodeConstHandle>(
          get_node().get_frame_value(representation_));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeConstHandles get_static_representation() const {
    try {
      return get_it<NodeConstHandle>(
          get_node().get_static_value(representation_));
    }
    RMF_DECORATOR_CATCH();
  }

  static std::string get_decorator_type_name() { return "RepresentationConst"; }
};
/** See also RepresentationFactory.
 */
class Representation : public RepresentationConst {
  friend class RepresentationFactory;
  Representation(NodeHandle nh, IntsKey representation)
      : RepresentationConst(nh, representation) {}

  template <class T>
  Ints set_it(const std::vector<T> &in) const {
    Ints ret;
    ret.reserve(in.size());
    RMF_FOREACH(T n, in) { ret.push_back(n.get_id().get_index()); }
    return ret;
  }
  Ints set_it(const NodeIDs &in) {
    Ints ret;
    ret.reserve(in.size());
    RMF_FOREACH(NodeID n, in) { ret.push_back(n.get_index()); }
    return ret;
  }

 public:
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  void set_representation(Ints v) {
    try {
      get_node().set_value(representation_, v);
    }
    RMF_DECORATOR_CATCH();
  }
  void set_frame_representation(Ints v) {
    try {
      get_node().set_frame_value(representation_, v);
    }
    RMF_DECORATOR_CATCH();
  }
  void set_static_representation(Ints v) {
    try {
      get_node().set_static_value(representation_, v);
    }
    RMF_DECORATOR_CATCH();
  }

  void set_representation(const NodeConstHandles &v) {
    try {
      set_representation(set_it(v));
    }
    RMF_DECORATOR_CATCH();
  }
  void set_frame_representation(const NodeConstHandles &v) {
    try {
      set_frame_representation(set_it(v));
    }
    RMF_DECORATOR_CATCH();
  }
  void set_static_representation(const NodeConstHandles &v) {
    try {
      set_static_representation(set_it(v));
    }
    RMF_DECORATOR_CATCH();
  }
  void set_representation(const NodeHandles &v) {
    try {
      set_representation(set_it(v));
    }
    RMF_DECORATOR_CATCH();
  }
  void set_frame_representation(const NodeHandles &v) {
    try {
      set_frame_representation(set_it(v));
    }
    RMF_DECORATOR_CATCH();
  }
  void set_static_representation(const NodeHandles &v) {
    try {
      set_static_representation(set_it(v));
    }
    RMF_DECORATOR_CATCH();
  }
#endif
  void set_representation(const NodeIDs &v) {
    try {
      set_representation(set_it(v));
    }
    RMF_DECORATOR_CATCH();
  }
  void set_frame_representation(const NodeIDs &v) {
    try {
      set_frame_representation(set_it(v));
    }
    RMF_DECORATOR_CATCH();
  }
  void set_static_representation(const NodeIDs &v) {
    try {
      set_static_representation(set_it(v));
    }
    RMF_DECORATOR_CATCH();
  }

  NodeHandles get_representation() const {
    try {
      return get_it<NodeHandle>(get_node().get_value(representation_));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeHandles get_frame_representation() const {
    try {
      return get_it<NodeHandle>(get_node().get_frame_value(representation_));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeHandles get_static_representation() const {
    try {
      return get_it<NodeHandle>(get_node().get_static_value(representation_));
    }
    RMF_DECORATOR_CATCH();
  }

  static std::string get_decorator_type_name() { return "Representation"; }
};

/** Create decorators of type Representation.
  */
class RepresentationFactory : public Factory {
  Category cat_;
  IntsKey representation_;

 public:
  RepresentationFactory(FileConstHandle fh)
      : cat_(fh.get_category("feature")),
        representation_(fh.get_key<IntsTag>(cat_, "representation")) {}
  RepresentationFactory(FileHandle fh)
      : cat_(fh.get_category("feature")),
        representation_(fh.get_key<IntsTag>(cat_, "representation")) {}
  /** Get a RepresentationConst for nh.*/
  RepresentationConst get(NodeConstHandle nh) const {
    RMF_USAGE_CHECK((nh.get_type() == RMF::FEATURE),
                    std::string("Bad node type. Got \"") +
                        boost::lexical_cast<std::string>(nh.get_type()) +
                        "\" in decorator type  Representation");
    return RepresentationConst(nh, representation_);
  }
  /** Get a Representation for nh.*/
  Representation get(NodeHandle nh) const {
    RMF_USAGE_CHECK((nh.get_type() == RMF::FEATURE),
                    std::string("Bad node type. Got \"") +
                        boost::lexical_cast<std::string>(nh.get_type()) +
                        "\" in decorator type  Representation");
    return Representation(nh, representation_);
  }
  /** Check whether nh has all the attributes required to be a
      RepresentationConst.*/
  bool get_is(NodeConstHandle nh) const {
    return (nh.get_type() == RMF::FEATURE) &&
           !nh.get_value(representation_).get_is_null();
  }
  bool get_is_static(NodeConstHandle nh) const {
    return (nh.get_type() == RMF::FEATURE) &&
           !nh.get_static_value(representation_).get_is_null();
  }
};
#ifndef RMF_DOXYGEN
struct RepresentationConstFactory : public RepresentationFactory {
  RepresentationConstFactory(FileConstHandle fh) : RepresentationFactory(fh) {}
  RepresentationConstFactory(FileHandle fh) : RepresentationFactory(fh) {}
};
#endif

} /* namespace decorator */
} /* namespace RMF */
RMF_DISABLE_WARNINGS

#endif /* RMF_DECORATOR_REPRESENTATION_H */
