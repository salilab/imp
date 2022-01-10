/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_KEY_FILTER_H
#define RMF_INTERNAL_KEY_FILTER_H

#include "RMF/config.h"
#include "RMF/internal/SharedData.h"
#include "backward_types.h"
#include "SharedDataAdaptor.h"
#include "RMF/log.h"

RMF_ENABLE_WARNINGS

namespace RMF {

namespace backends {

template <class SD>
class KeyFilter : public SharedDataAdaptor<SD> {
  typedef SharedDataAdaptor<SD> P;
  backward_types::IndexKeys index_keys_;
  FloatKeys float_keys_;
  FloatsKeys floats_keys_;

 public:
  KeyFilter(SD* sd) : P(sd) {}
  void add_index_key(Category cat, std::string name) {
    RMF_FOREACH(backward_types::IndexKey k,
                P::sync_->get_keys(cat, backward_types::IndexTraits())) {
      if (P::sync_->get_name(k) == name) {
        index_keys_.push_back(k);
        std::sort(index_keys_.begin(), index_keys_.end());
        break;
      }
    }
  }
  void add_float_key(Category cat, std::string name) {
    RMF_FOREACH(FloatKey k, P::sync_->get_keys(cat, FloatTraits())) {
      if (P::sync_->get_name(k) == name) {
        float_keys_.push_back(k);
        std::sort(float_keys_.begin(), float_keys_.end());
        break;
      }
    }
  }
  void add_floats_key(Category cat, std::string name) {
    RMF_FOREACH(FloatsKey k, P::sync_->get_keys(cat, FloatsTraits())) {
      if (P::sync_->get_name(k) == name) {
        floats_keys_.push_back(k);
        std::sort(floats_keys_.begin(), floats_keys_.end());
        break;
      }
    }
  }
  template <class Traits>
  std::vector<ID<Traits> > get_keys(Category cat, Traits) {
    return P::sync_->get_keys(cat, Traits());
  }
  backward_types::IndexKeys get_keys(Category cat,
                                     backward_types::IndexTraits) {
    backward_types::IndexKeys all =
        P::sync_->get_keys(cat, backward_types::IndexTraits());
    std::sort(all.begin(), all.end());
    backward_types::IndexKeys ret;
    std::set_difference(all.begin(), all.end(), index_keys_.begin(),
                        index_keys_.end(), std::back_inserter(ret));
    return ret;
  }
  FloatKeys get_keys(Category cat, FloatTraits) {
    FloatKeys all = P::sync_->get_keys(cat, FloatTraits());
    std::sort(all.begin(), all.end());
    FloatKeys ret;
    std::set_difference(all.begin(), all.end(), float_keys_.begin(),
                        float_keys_.end(), std::back_inserter(ret));
    return ret;
  }
  FloatsKeys get_keys(Category cat, FloatsTraits) {
    FloatsKeys all = P::sync_->get_keys(cat, FloatsTraits());
    std::sort(all.begin(), all.end());
    FloatsKeys ret;
    std::set_difference(all.begin(), all.end(), floats_keys_.begin(),
                        floats_keys_.end(), std::back_inserter(ret));
    return ret;
  }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_KEY_FILTER_H */
