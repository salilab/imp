/**
 *  \file RMF/decorator/alternatives.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_DECORATORS_ALTERNATIVES_H
#define RMF_DECORATORS_ALTERNATIVES_H

#include <RMF/config.h>
#include <RMF/infrastructure_macros.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/Decorator.h>
#include <RMF/constants.h>
#include <RMF/Enum.h>
#include <RMF/Vector.h>

RMF_ENABLE_WARNINGS
namespace RMF {
namespace decorator {

/** See also Alternatives and AlternativesFactory.
  */
class RMFEXPORT AlternativesConst : public Decorator {
  friend class AlternativesFactory;
  friend class Alternatives;
  IntsKey types_key_;
  IntsKey roots_key_;
  AlternativesConst(NodeConstHandle nh, IntsKey types_key, IntsKey roots_key)
      : Decorator(nh), types_key_(types_key), roots_key_(roots_key) {}
  NodeID get_alternative_impl(RepresentationType type, float resolution) const;
  NodeIDs get_alternatives_impl(RepresentationType type) const;

 public:
  /** Get the alternative root that best matches the criteria. */
  NodeConstHandle get_alternative(RepresentationType type,
                                  double resolution) const;

  /** Get the type of the representation with the given node id. */
  RepresentationType get_representation_type(NodeID id) const;

  RepresentationType get_representation_type(NodeConstHandle id) const {
    return get_representation_type(id.get_id());
  }

  /** Get all the alternatives (including this node).

     You can use get_resolution() and get_representation_type() to get info
     about them. */
  NodeConstHandles get_alternatives(RepresentationType type) const;

  static std::string get_decorator_type_name() { return "AlternativesConst"; }
  RMF_SHOWABLE(AlternativesConst, "Alternatives: " << get_node());
};

/** See also AlternativesConst and AlternativesFactory.
  */
class RMFEXPORT Alternatives : public AlternativesConst {
  friend class AlternativesFactory;
  Alternatives(NodeHandle nh, IntsKey types_key, IntsKey roots_key);

 public:
  void add_alternative(NodeHandle root, RepresentationType type);

  static std::string get_decorator_type_name() { return "Alternatives"; }
};

/** Create decorators of type Alternatives.

     See also Alternatives and AlternativesFactory.
  */
class RMFEXPORT AlternativesFactory : public Factory {
  Category cat_;
  IntsKey types_key_;
  IntsKey roots_key_;

 public:
  AlternativesFactory(FileConstHandle fh);
  AlternativesFactory(FileHandle fh);

  Alternatives get(NodeHandle nh) const {
    return Alternatives(nh, types_key_, roots_key_);
  }
  AlternativesConst get(NodeConstHandle nh) const {
    return AlternativesConst(nh, types_key_, roots_key_);
  }
  bool get_is(NodeConstHandle nh) const { return nh.get_has_value(types_key_); }
  bool get_is_static(NodeConstHandle nh) const {
    return nh.get_has_value(types_key_);
  }
  RMF_SHOWABLE(AlternativesFactory, "AlternativesFactory");
};

#ifndef RMF_DOXYGEN
struct AlternativesConstFactory : public AlternativesFactory {
  AlternativesConstFactory(FileConstHandle fh) : AlternativesFactory(fh) {}
  AlternativesConstFactory(FileHandle fh) : AlternativesFactory(fh) {}
};
#endif

/** Return the canonical resolution of the subtree. */
RMFEXPORT double get_resolution(NodeConstHandle root);

/** Return a list of (clustered) resolution levels available in the subtree.

    Use this, for example, when making a slider for display.
*/
RMFEXPORT Floats get_resolutions(NodeConstHandle root,
                                 RepresentationType type = PARTICLE,
                                 double accuracy = 0);

} /* namespace decorator */
} /* namespace RMF */
RMF_DISABLE_WARNINGS

#endif /* RMF_DECORATORS_ALTERNATIVES_H */
