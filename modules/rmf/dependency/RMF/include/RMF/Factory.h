/**
 *  \file RMF/Factory.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_FACTORY_H
#define RMF_FACTORY_H

#include <RMF/config.h>
#include "infrastructure_macros.h"

RMF_ENABLE_WARNINGS

namespace RMF {

/** Factories in RMF allow one to create Decorator objects as well
    as check whether a given node has the values needed for a particular
    decorator type in a given frame.
 */
template <class HandleType>
class Factory {
protected:
  Factory() {
  }
public:
#ifdef RMF_DOXYGEN
  /** \name Methods
      Each factory will have the following methods, where Decorator is
      the name of the decorator type being produced.
      @{
   */
  //! Return if the node has the needed attributes for the Decorator
  bool get_is(NodeConstHandle nh) const;
  //! Return the decorator
  Decorator get(NodeConstHandle nh) const;
  /** @} */
#endif

  RMF_SHOWABLE(Factory,
               "Factory");
};

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_FACTORY_H */
