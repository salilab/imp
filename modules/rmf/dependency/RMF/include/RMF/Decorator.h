/**
 *  \file RMF/Decorator.h
 *  \brief Mostly empty base classes for decorators and factories.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_DECORATOR_H
#define RMF_DECORATOR_H

#include "RMF/config.h"
#include "infrastructure_macros.h"
#include "ID.h"
#include "internal/SharedData.h"
#include "NodeConstHandle.h"
#include "NodeHandle.h"
#include <boost/shared_ptr.hpp>

RMF_ENABLE_WARNINGS
namespace RMF {

//! The base class for decorators.
/** Decorators in RMF provide high level routines to manipulate attributes
    of nodes in the hierarchy. They are created by an associated Factory.

    See [Decorators and attributes](\ref decoratorsattributes) for more info.
 */
class Decorator {
 private:
  NodeID id_;
  boost::shared_ptr<internal::SharedData> data_;

 protected:
  Decorator(NodeConstHandle handle)
      : id_(handle.get_id()), data_(handle.get_shared_data()) {};

  NodeHandle get_node() const { return NodeHandle(id_, data_); }

 public:
  RMF_SHOWABLE(Decorator, get_node().get_name());
};
//! The base class for Factories
/** This common base class for all factories is here for organizational
 * purposes. It doesn't provide any functionality. */
class Factory {};

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_DECORATOR_H */
