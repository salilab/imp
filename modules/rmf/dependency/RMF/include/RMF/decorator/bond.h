/**
 *  \file RMF/decorator/bond.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_DECORATOR_BOND_H
#define RMF_DECORATOR_BOND_H

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

/** See also Bond and BondFactory.
  */
class BondConst : public Decorator {
  friend class BondFactory;
  friend class Bond;
  IntKey bonded_0_;
  IntKey bonded_1_;
  BondConst(NodeConstHandle nh, IntKey bonded_0, IntKey bonded_1)
      : Decorator(nh), bonded_0_(bonded_0), bonded_1_(bonded_1) {}

 public:
  NodeConstHandle get_bonded_0() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_value(bonded_0_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeConstHandle get_frame_bonded_0() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_frame_value(bonded_0_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeConstHandle get_static_bonded_0() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_static_value(bonded_0_)));
    }
    RMF_DECORATOR_CATCH();
  }

  NodeConstHandle get_bonded_1() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_value(bonded_1_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeConstHandle get_frame_bonded_1() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_frame_value(bonded_1_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeConstHandle get_static_bonded_1() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_static_value(bonded_1_)));
    }
    RMF_DECORATOR_CATCH();
  }

  static std::string get_decorator_type_name() { return "BondConst"; }
  RMF_SHOWABLE(BondConst, "Bond: " << get_node());
};
/** See also BondFactory.
 */
class Bond : public BondConst {
  friend class BondFactory;
  Bond(NodeHandle nh, IntKey bonded_0, IntKey bonded_1)
      : BondConst(nh, bonded_0, bonded_1) {}

 public:
#ifndef RMF_DOXYGEN
  void set_bonded_0(Int v) {
    try {
      get_node().set_value(bonded_0_, v);
    }
    RMF_DECORATOR_CATCH();
  }
  void set_frame_bonded_0(Int v) {
    try {
      get_node().set_frame_value(bonded_0_, v);
    }
    RMF_DECORATOR_CATCH();
  }
  void set_static_bonded_0(Int v) {
    try {
      get_node().set_static_value(bonded_0_, v);
    }
    RMF_DECORATOR_CATCH();
  }

  void set_bonded_1(Int v) {
    try {
      get_node().set_value(bonded_1_, v);
    }
    RMF_DECORATOR_CATCH();
  }
  void set_frame_bonded_1(Int v) {
    try {
      get_node().set_frame_value(bonded_1_, v);
    }
    RMF_DECORATOR_CATCH();
  }
  void set_static_bonded_1(Int v) {
    try {
      get_node().set_static_value(bonded_1_, v);
    }
    RMF_DECORATOR_CATCH();
  }
#endif

  NodeHandle get_bonded_0() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_value(bonded_0_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeHandle get_frame_bonded_0() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_frame_value(bonded_0_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeHandle get_static_bonded_0() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_static_value(bonded_0_)));
    }
    RMF_DECORATOR_CATCH();
  }

  NodeHandle get_bonded_1() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_value(bonded_1_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeHandle get_frame_bonded_1() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_frame_value(bonded_1_)));
    }
    RMF_DECORATOR_CATCH();
  }
  NodeHandle get_static_bonded_1() const {
    try {
      return get_node().get_file().get_node(
          NodeID(get_node().get_static_value(bonded_1_)));
    }
    RMF_DECORATOR_CATCH();
  }

  void set_bonded_0(NodeConstHandle v) {
    try {
      get_node().set_value(bonded_0_, v.get_id().get_index());
    }
    RMF_DECORATOR_CATCH();
  }
  void set_frame_bonded_0(NodeConstHandle v) {
    try {
      get_node().set_frame_value(bonded_0_, v.get_id().get_index());
    }
    RMF_DECORATOR_CATCH();
  }
  void set_static_bonded_0(NodeConstHandle v) {
    try {
      get_node().set_static_value(bonded_0_, v.get_id().get_index());
    }
    RMF_DECORATOR_CATCH();
  }

  void set_bonded_1(NodeConstHandle v) {
    try {
      get_node().set_value(bonded_1_, v.get_id().get_index());
    }
    RMF_DECORATOR_CATCH();
  }
  void set_frame_bonded_1(NodeConstHandle v) {
    try {
      get_node().set_frame_value(bonded_1_, v.get_id().get_index());
    }
    RMF_DECORATOR_CATCH();
  }
  void set_static_bonded_1(NodeConstHandle v) {
    try {
      get_node().set_static_value(bonded_1_, v.get_id().get_index());
    }
    RMF_DECORATOR_CATCH();
  }

  static std::string get_decorator_type_name() { return "Bond"; }
};

/** Create decorators of type Bond.
  */
class BondFactory : public Factory {
  Category cat_;
  IntKey bonded_0_;
  IntKey bonded_1_;

 public:
  BondFactory(FileConstHandle fh)
      : cat_(fh.get_category("physics")),
        bonded_0_(fh.get_key<IntTag>(cat_, "bonded 0")),
        bonded_1_(fh.get_key<IntTag>(cat_, "bonded 1")) {}
  BondFactory(FileHandle fh)
      : cat_(fh.get_category("physics")),
        bonded_0_(fh.get_key<IntTag>(cat_, "bonded 0")),
        bonded_1_(fh.get_key<IntTag>(cat_, "bonded 1")) {}
  /** Get a BondConst for nh.*/
  BondConst get(NodeConstHandle nh) const {
    RMF_USAGE_CHECK((nh.get_type() == RMF::BOND),
                    std::string("Bad node type. Got \"") +
                        boost::lexical_cast<std::string>(nh.get_type()) +
                        "\" in decorator type  Bond");
    return BondConst(nh, bonded_0_, bonded_1_);
  }
  /** Get a Bond for nh.*/
  Bond get(NodeHandle nh) const {
    RMF_USAGE_CHECK((nh.get_type() == RMF::BOND),
                    std::string("Bad node type. Got \"") +
                        boost::lexical_cast<std::string>(nh.get_type()) +
                        "\" in decorator type  Bond");
    return Bond(nh, bonded_0_, bonded_1_);
  }
  /** Check whether nh has all the attributes required to be a
      BondConst.*/
  bool get_is(NodeConstHandle nh) const {
    return (nh.get_type() == RMF::BOND) &&
           !nh.get_value(bonded_0_).get_is_null();
  }
  bool get_is_static(NodeConstHandle nh) const {
    return (nh.get_type() == RMF::BOND) &&
           !nh.get_static_value(bonded_0_).get_is_null() &&
           !nh.get_static_value(bonded_1_).get_is_null();
  }
  RMF_SHOWABLE(BondFactory, "BondFactory");
};
#ifndef RMF_DOXYGEN
struct BondConstFactory : public BondFactory {
  BondConstFactory(FileConstHandle fh) : BondFactory(fh) {}
  BondConstFactory(FileHandle fh) : BondFactory(fh) {}
};
#endif

} /* namespace decorator */
} /* namespace RMF */
RMF_DISABLE_WARNINGS

#endif /* RMF_DECORATOR_BOND_H */
