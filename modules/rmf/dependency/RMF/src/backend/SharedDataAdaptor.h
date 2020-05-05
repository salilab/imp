/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_ADAPTOR_H
#define RMF_INTERNAL_SHARED_DATA_ADAPTOR_H

#include "RMF/config.h"
#include "RMF/internal/SharedData.h"
#include "RMF/log.h"
#include <boost/array.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {

namespace backends {

template <class SD>
class SharedDataAdaptor {
 protected:
  SD* sync_;

 public:
  SharedDataAdaptor(SD* sd) : sync_(sd) {}
  unsigned int get_number_of_nodes() const {
    return sync_->get_number_of_nodes();
  }
  template <class Traits>
  typename Traits::Type get_loaded_value(NodeID n, ID<Traits> k) const {
    return sync_->get_loaded_value(n, k);
  }
  template <class Traits>
  typename Traits::Type get_static_value(NodeID n, ID<Traits> k) const {
    return sync_->get_static_value(n, k);
  }
  template <class T>
  std::string get_name(T k) const {
    return sync_->get_name(k);
  }
  template <class Traits>
  std::vector<ID<Traits> > get_keys(Category c, Traits) const {
    return sync_->get_keys(c, Traits());
  }
  template <class Traits>
  void set_static_value(NodeID n, ID<Traits> k,
                        typename Traits::ArgumentType d) {
    sync_->set_static_value(n, k, d);
  }
  template <class Traits>
  void set_loaded_value(NodeID n, ID<Traits> k,
                        typename Traits::ArgumentType d) {
    sync_->set_loaded_value(n, k, d);
  }
  template <class Traits>
  ID<Traits> get_key(Category c, std::string name, Traits) {
    return sync_->get_key(c, name, Traits());
  }
  Category get_category(std::string name) { return sync_->get_category(name); }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_ADAPTOR_H */
