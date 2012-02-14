/**
 *  \file RMF/utility.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_UTILITY_H
#define IMPLIBRMF_UTILITY_H

#include "RMF_config.h"
#include "internal/intrusive_ptr_object.h"
namespace RMF {

/** Lazily construct a decorator factor when it is actually used.
    Using this, one can avoid adding attributes and categories
    to files that are not used why avoiding constructing the
    factories more than once.
*/
template <class Factory>
class LazyFactory {
  class Data: public boost::intrusive_ptr_object {
    bool initialized_;
    boost::scoped_ptr<Factory> f_;
    typename Factory::File fh_;
   public:
    Data(typename Factory::File fh): initialized_(false), fh_(fh){}
    const Factory&get() {
      if (!initialized_) {
        initialized_=true;
        f_.reset(new Factory(fh_));
      }
      return *f_;
    }
  };
  boost::intrusive_ptr<Data> data_;
 public:
  IMP_RMF_SHOWABLE(LazyFactor, "lazy factory");
  LazyFactory(typename Factory::File fh):
      data_(new Data(fh)) {}
  typename Factory::Decorator get(NodeHandle nh, int frame=0) {
    return data_->get().get(nh, frame);
  }
  bool get_is(NodeHandle nh, int frame=0) {
    return data_->get().get_is(nh, frame);
  }
};


} /* namespace RMF */

#endif /* IMPLIBRMF_UTILITY_H */
