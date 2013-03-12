/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_H
#define RMF_INTERNAL_SHARED_DATA_H

#include <RMF/config.h>
#include "../Key.h"
#include "../types.h"
#include "../names.h"
#include "../infrastructure_macros.h"
#include "map.h"
#include "set.h"
#include <boost/cstdint.hpp>
#include <boost/any.hpp>
#include <algorithm>

#include <boost/shared_ptr.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {
template <class P>
inline uintptr_t get_uint(const P *p) {
  return reinterpret_cast<uintptr_t>(p);
}
template <class P>
inline uintptr_t get_uint(boost::shared_ptr<P> p) {
  return reinterpret_cast<uintptr_t>(p.get());
}
inline uintptr_t get_uint(NodeID id) {
  return id.get_index();
}

namespace internal {



#define RMF_SHARED_TYPE(lcname, Ucname, PassValue, ReturnValue,       \
                        PassValues, ReturnValues)                     \
  /** Return a value or the null value.*/                             \
  virtual Ucname##Traits::Type get_value(unsigned int node,           \
                                         Key<Ucname##Traits> k)       \
  const = 0;                                                          \
  virtual Ucname##Traits::Type get_value_frame(unsigned int node,     \
                                               Key<Ucname##Traits> k) \
  const = 0;                                                          \
  virtual Ucname##Traits::Types                                       \
  get_values(unsigned int node,                                       \
             const std::vector<Key<Ucname##Traits> >&k) const {       \
    Ucname##Traits::Types ret(k.size());                              \
    for (unsigned int i = 0; i < k.size(); ++i) {                     \
      ret[i] = get_value(node, k[i]);                                 \
    }                                                                 \
    return ret;                                                       \
  }                                                                   \
  virtual Ucname##Traits::Types                                       \
  get_all_values(unsigned int node,                                   \
                 Key<Ucname##Traits> k) const {                       \
    SharedData *sd= const_cast<SharedData*>(this);                    \
    unsigned int nf = get_number_of_frames();                         \
    int start_frame = get_current_frame();                            \
    Ucname##Traits::Types ret(nf);                                    \
    for (unsigned int i = 0; i < nf; ++i) {                           \
      sd->set_current_frame(i);                                       \
      ret[i] = get_value(node, k);                                    \
    }                                                                 \
    sd->set_current_frame(start_frame);                               \
    return ret;                                                       \
  }                                                                   \
  virtual bool get_has_frame_value(unsigned int node,                 \
                                   Key<Ucname##Traits> k) const = 0;  \
  virtual void set_value(unsigned int node,                           \
                         Key<Ucname##Traits> k,                       \
                         Ucname##Traits::Type v) = 0;                 \
  virtual void set_value_frame(unsigned int node,                     \
                               Key<Ucname##Traits> k,                 \
                               Ucname##Traits::Type v) = 0;           \
  virtual void set_values(unsigned int node,                          \
                          const std::vector<Key<Ucname##Traits> > &k, \
                          const Ucname##Traits::Types v) {            \
    for (unsigned int i = 0; i < k.size(); ++i) {                     \
      set_value(node, k[i], v[i]);                                    \
    }                                                                 \
  }                                                                   \
  virtual std::vector<Key<Ucname##Traits> >                           \
  get_##lcname##_keys(Category category) = 0;                         \
  virtual Category                                                    \
  get_category(Key<Ucname##Traits> k) const = 0;                      \
  virtual Key<Ucname##Traits>                                         \
  get_##lcname##_key(Category category,                               \
                     std::string name) = 0;                           \
  virtual std::string get_name(Key<Ucname##Traits> k) const = 0

/**
   Base class for wrapping all the file handles, caches, etc. for
   open RMF file handles, and to manage the associations between
   external objects and nodes in the RMF hierarchy

   Note this class serves as an internal interface to RMS file handling
   with an almost one-to-one mapping between most of its functions and
   exposed functions
 */
class SharedData: public boost::intrusive_ptr_object {
  std::vector<boost::any> association_;
  std::vector<uintptr_t> back_association_value_;
  map<uintptr_t, int> back_association_;
  map<int, boost::any> user_data_;
  int valid_;
  int cur_frame_;
  std::string path_;
protected:
  SharedData(std::string path);
public:
  std::string get_file_path() const {
    return path_;
  }
  int get_current_frame() const {
    return cur_frame_;
  }
  virtual void set_current_frame(int frame);

  RMF_FOREACH_TYPE(RMF_SHARED_TYPE);
  void audit_key_name(std::string name) const;
  void audit_node_name(std::string name) const;
  template <class T>
  void set_user_data(int i, const T&d) {
    user_data_[i] = boost::any(d);
  }
  bool get_has_user_data(int i) const {
    return user_data_.find(i) != user_data_.end();
  }
  template <class T>
  T get_user_data(int i) const {
    RMF_USAGE_CHECK(user_data_.find(i)
                    != user_data_.end(),
                    "No such data found");
    try {
      return boost::any_cast<T>(user_data_.find(i)->second);
    } catch (boost::bad_any_cast) {
      RMF_THROW(Message("Type mismatch when recovering user data"),
                UsageException);
    }
    RMF_NO_RETURN(T);
  }
  template <class T>
  void set_association(int id, const T& d, bool overwrite) {
    if (association_.size() <= static_cast<unsigned int>(id)) {
      association_.resize(id + 1, boost::any());
      back_association_value_.resize(id + 1);
    }
    RMF_USAGE_CHECK(overwrite || association_[id].empty(),
                    "Associations can only be set once");
    if (overwrite && !association_[id].empty()) {
      uintptr_t v = back_association_value_[id];
      back_association_.erase(v);
    }
    uintptr_t v = get_uint(d);
    back_association_value_[id] = v;
    association_[id] = boost::any(d);
    RMF_USAGE_CHECK(back_association_.find(v)
                    == back_association_.end(),
                    "Collision on association keys.");
    back_association_[v] = id;
  }
  template <class T>
  bool get_has_associated_node(const T& v) const {
    return back_association_.find(get_uint(v)) != back_association_.end();
  }
  boost::any get_association(int id) const {
    RMF_USAGE_CHECK(static_cast<unsigned int>(id) < association_.size(),
                    std::string("Unassociated id ") + get_name(id));
    try {
      return association_[id];
    } catch (boost::bad_any_cast) {
      RMF_THROW(Message("Type mismatch when recovering node data"),
                UsageException);
    }
    RMF_NO_RETURN(boost::any);
  }
  bool get_has_association(int id) const {
    if (id >= static_cast<int>(association_.size())) return false;
    return !association_[id].empty();
  }
  template <class T>
  int get_associated_node(const T &d) const {
    return back_association_.find(get_uint(d))->second;
  }
  virtual void flush() = 0;
  std::string get_file_name() const;

  virtual unsigned int get_number_of_frames() const = 0;

  //SharedData(HDF5Group g, bool create);
  virtual ~SharedData();
  virtual std::string get_name(unsigned int node) const = 0;
  virtual unsigned int get_type(unsigned int node) const = 0;

  virtual int add_child(int node, std::string name, int t) = 0;
  virtual void add_child(int node, int child_node) = 0;
  virtual Ints get_children(int node) const = 0;

  virtual int add_child_frame(int node, std::string name, int t) = 0;
  virtual void add_child_frame(int node, int child_node) = 0;
  virtual Ints get_children_frame(int node) const = 0;


  virtual void save_frames_hint(int i) = 0;

  virtual Categories get_categories() const = 0;
  virtual Category get_category(std::string name) = 0;
  virtual std::string get_category_name(Category kc) const = 0;
  virtual std::string get_description() const = 0;
  virtual void set_description(std::string str) = 0;
  virtual std::string get_producer() const = 0;
  virtual void set_producer(std::string str) = 0;
  virtual std::string get_frame_name(int i) const = 0;
  virtual bool get_supports_locking() const {
    return false;
  }
  virtual bool set_is_locked(bool) {
    return false;
  }
  virtual void reload() = 0;
};

template <class Traits>
class GenericSharedData {
};
template <class Traits>
class ConstGenericSharedData {
};

#define RMF_GENERIC_SHARED(lcname, Ucname, PassValue, ReturnValue, \
                           PassValues, ReturnValues)               \
  template <>                                                      \
  class ConstGenericSharedData<Ucname##Traits> {                   \
public:                                                            \
    typedef Key<Ucname##Traits> K;                                 \
    typedef std::vector<K > Ks;                                    \
  };                                                               \
  template <>                                                      \
  class GenericSharedData<Ucname##Traits> {                        \
public:                                                            \
    typedef Key<Ucname##Traits> K;                                 \
    typedef std::vector<K > Ks;                                    \
    static Ks get_keys(  SharedData * p,                           \
                         Category category) {                      \
      return p->get_##lcname##_keys(category);                     \
    }                                                              \
    static K get_key( SharedData * p,                              \
                      Category category,                           \
                      std::string name) {                          \
      return p->get_##lcname##_key(category,                       \
                                   name);                          \
    }                                                              \
  };

RMF_FOREACH_TYPE(RMF_GENERIC_SHARED);


/**
   Construct shared data for the RMF file in 'path', either creating or
   the file or opening an existing file according to the value of 'create'.
   Note on internal implementation - stores results in internal cache

   @param path path to RMF file
   @param create whether to create the file or just open it
   @exception IOException if couldn't create file or unsupported file format
 */
RMFEXPORT SharedData* create_shared_data(std::string path, bool create);

/**
   Construct shared data for the RMF file in 'path' in read only mode
   Note on internal implementation - stores results in internal cache

   @param path path to RMF file
   @param create whether to create the file or just open it
   @exception RMF::IOException if couldn't open file or unsupported file
              format
 */
RMFEXPORT SharedData* create_read_only_shared_data(std::string path);



RMFEXPORT SharedData*
create_shared_data_in_buffer(std::string &buffer,
                             bool        create);

RMFEXPORT SharedData*
create_read_only_shared_data_from_buffer(const std::string& buffer);


// needed for correctness imposed by clang as the functions must be visible
// by ADL (or declared before the usage which is almost impossible to achieve
// as we can't control whether boost intrusive_ptr.hpp is included before
// us or not
inline void intrusive_ptr_add_ref(SharedData* a)
{
  (a)->add_ref();
}


inline void intrusive_ptr_release(SharedData *a)
{
  bool del = (a)->release();
  if (del) {
    delete a;
  }
}


}   // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_H */
