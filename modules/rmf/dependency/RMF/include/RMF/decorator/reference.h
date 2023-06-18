/**
 *  \file RMF/decorator/reference.h
 *  \brief Add a pointer from a node to another reference node.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_DECORATOR_REFERENCE_H
#define RMF_DECORATOR_REFERENCE_H

#include <RMF/config.h>
#include <RMF/infrastructure_macros.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/Decorator.h>
#include <RMF/constants.h>
#include <RMF/Vector.h>
#include <array>
#include <boost/lexical_cast.hpp>

RMF_ENABLE_WARNINGS
namespace RMF {
namespace decorator {

/** See also Reference and ReferenceFactory.
  */
class ReferenceConst : public Decorator {
  friend class ReferenceFactory;
  friend class Reference;
  IntKey reference_;
  ReferenceConst(NodeConstHandle nh, IntKey reference)
      : Decorator(nh), reference_(reference) {}

 public:
  NodeConstHandle get_reference() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_value(reference_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeConstHandle get_frame_reference() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_frame_value(reference_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeConstHandle get_static_reference() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_static_value(reference_)));
    }
    RMF_DECORATOR_CATCH();
  }

  static std::string get_decorator_type_name() { return "ReferenceConst"; }
  RMF_SHOWABLE(ReferenceConst, "Reference: " << get_node());
};
/** See also ReferenceFactory.
 */
class Reference : public ReferenceConst {
  friend class ReferenceFactory;
  Reference(NodeHandle nh, IntKey reference)
      : ReferenceConst(nh, reference) {}

 public:

  NodeHandle get_reference() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_value(reference_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeHandle get_frame_reference() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_frame_value(reference_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeHandle get_static_reference() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_static_value(reference_)));
    }
    RMF_DECORATOR_CATCH();
  }

  void set_reference(NodeConstHandle v) {
    try {
      get_node().set_value(reference_, v.get_id().get_index());
    }
    RMF_DECORATOR_CATCH();
  }
  void set_frame_reference(NodeConstHandle v) {
    try {
      get_node().set_frame_value(reference_, v.get_id().get_index());
    }
    RMF_DECORATOR_CATCH();
  }
  void set_static_reference(NodeConstHandle v) {
    try {
      get_node().set_static_value(reference_, v.get_id().get_index());
    }
    RMF_DECORATOR_CATCH();
  }

  static std::string get_decorator_type_name() { return "Reference"; }
};

/** Create decorators of type Reference.
  */
class ReferenceFactory : public Factory {
  Category cat_;
  IntKey reference_;

 public:
  ReferenceFactory(FileConstHandle fh)
      : cat_(fh.get_category("sequence")),
        reference_(fh.get_key<IntTag>(cat_, "reference")) {}
  ReferenceFactory(FileHandle fh)
      : cat_(fh.get_category("sequence")),
        reference_(fh.get_key<IntTag>(cat_, "reference")) {}
  /** Get a ReferenceConst for nh.*/
  ReferenceConst get(NodeConstHandle nh) const {
    RMF_USAGE_CHECK((nh.get_type() == RMF::REPRESENTATION),
                    std::string("Bad node type. Got \"") +
                        boost::lexical_cast<std::string>(nh.get_type()) +
                        "\" in decorator type Reference");
    return ReferenceConst(nh, reference_);
  }
  /** Get a Reference for nh.*/
  Reference get(NodeHandle nh) const {
    RMF_USAGE_CHECK((nh.get_type() == RMF::REPRESENTATION),
                    std::string("Bad node type. Got \"") +
                        boost::lexical_cast<std::string>(nh.get_type()) +
                        "\" in decorator type Reference");
    return Reference(nh, reference_);
  }
  /** Check whether nh has all the attributes required to be a
      ReferenceConst.*/
  bool get_is(NodeConstHandle nh) const {
    return (nh.get_type() == RMF::REPRESENTATION) &&
           !nh.get_value(reference_).get_is_null();
  }
  bool get_is_static(NodeConstHandle nh) const {
    return (nh.get_type() == RMF::REPRESENTATION) &&
           !nh.get_static_value(reference_).get_is_null();
  }
  RMF_SHOWABLE(ReferenceFactory, "ReferenceFactory");
};

} /* namespace decorator */
} /* namespace RMF */
RMF_DISABLE_WARNINGS

#endif /* RMF_DECORATOR_REFERENCE_H */
