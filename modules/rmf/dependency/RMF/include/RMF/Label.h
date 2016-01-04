/**
 *  \file RMF/Label.h
 *  \brief Declaration of RMF::Label.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_LABEL_H
#define RMF_LABEL_H

#include "RMF/config.h"
#include "NodeHandle.h"

RMF_ENABLE_WARNINGS
namespace RMF {

//! A decorator-like class to mark certain nodes (eg molecule bounaries)
/** Labels mark a node of the hierarchy as having a property, but, unlike
    Decorator nodes, don't have any other associated data.
 */
class Label {
  IntKey k_;
  std::string name_;

 protected:
  template <class FH>
  Label(FH fh, std::string category, std::string name)
      : name_(name) {
    Category cat = fh.get_category(category);
    k_ = fh.template get_key<IntTag>(cat, name);
  }

 public:
  Label() {}
  bool get_is(NodeConstHandle nh) const { return nh.get_has_value(k_); }
  void set_is(NodeHandle nh) const { nh.set_value(k_, 1); }
  RMF_SHOWABLE(Label, name_);
};

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_LABEL_H */
