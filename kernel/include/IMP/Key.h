/**
 *  \file Key.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_KEY_H
#define __IMP_KEY_H

#include "macros.h"
#include "exception.h"

#include <map>
#include <vector>


/**
   \internal
   \page keys How Keys work in IMP

   The keys in IMP maintain a cached mapping between strings and indexes.
   This mapping is global--that is all IMP Models and Particles in the
   same program use the same mapping.

   The mapping uses a static table which is defined with the
   IMP_DEFINE_KEY_TYPE macro. As a result, the macro must be used in exactly
   one .o. If it appears more than once or that .o is linked in multiple times,
   then bad things can happen.

   Since the order of initialization of static data is undefined
   between .os, it is important that no other static data not in the
   same .o uses the key data. Specifically, typedes defined by
   IMP_DECLARE_KEY_TYPE should never be statically initialized. While
   this is annoying, statically initializing them would be bad
   practice anyway, as unused attribute keys would still be mapped to
   indices and would make the set of indices less dense.
 */

// Swig chokes on the specialization
#ifndef SWIG
//! \internal
#define IMP_KEY_DATA_HOLDER(Name, Tag)                                  \
  namespace internal {                                                  \
  template <>                                                           \
  struct IMPDLLEXPORT KeyDataHolder<Tag> {                              \
    static KeyData data;                                                \
  };                                                                    \
  }
#else
#define IMP_KEY_DATA_HOLDER(Name, Tag)
#endif


/**
   Define a new key type. There must be an accompanying IMP_DEFINE_KEY_TYPE
   located in some .o file. This macro should be used in the IMP namespace.

   It defines two public types Name, which is an instantiation of KeyBase and
   Names which is a vector of Name.

   \param[in] Name The name for the new type.
   \param[in] Tag A class which is unique for this type. For attributes
   use the type of the attributes. For other Keys, declare an empty
   class with a unique name and use it.
 */
#define IMP_DECLARE_KEY_TYPE(Name, Tag)                                 \
  IMP_KEY_DATA_HOLDER(Name, Tag);                                       \
  typedef KeyBase<Tag> Name;                                                \
  typedef std::vector<Name> Name##s


/** This must occur in exactly one .o in the internal namespace. Should 
 be used in the IMP namespace.*/
#define IMP_DEFINE_KEY_TYPE(Name, Tag)                  \
  namespace internal {                                  \
    KeyData KeyDataHolder<Tag>::data;                   \
  }


namespace IMP
{

namespace internal
{

/** The data concerning a particular type of key.
    \internal
 */
struct IMPDLLEXPORT KeyData
{
  typedef std::map<std::string, int> Map;
  typedef std::vector<std::string> RMap;

  void show(std::ostream &out= std::cout) const;
  KeyData();
  void assert_is_initialized() const;
  unsigned int add_key(std::string str) {
    unsigned int i= map_.size();
    map_[str]=i;
    rmap_.push_back(str);
    return i;
  }

  const Map &get_map() const {return map_;}
  const RMap &get_rmap() const {return rmap_;}

private:
  double heuristic_;
  Map map_;
  RMap rmap_;

};

/** A dummy class. Actual keys types create specializations

    \internal
*/
template <class T>
struct KeyDataHolder {};

} // namespace internal



//! A base class for  Keys
/** This class does internal caching of the strings to accelerate the
    name lookup. It is better to create an Key and reuse it
    rather than recreate it many times from strings.

    If you use this with a new type, you must add a new definition of
    attribute_table_index. Yes, this is an evil hack, but I couldn't
    get linking to work with static members of the template class.
 */
template <class T>
class KeyBase
{
  int str_;

  typedef T Type;

  bool is_default() const;

  static const internal::KeyData::Map& get_map();
  static const internal::KeyData::RMap& get_rmap();

public:
  static const std::string &get_string(int i);

  typedef KeyBase<T> This;

  //! make a default (uninitalized) key
  KeyBase(): str_(-1) {}


  //! Generate a key from the given string
  KeyBase(const char *c) {
    std::string sc(c);
    if (get_map().find(sc) == get_map().end()) {
      str_= internal::KeyDataHolder<T>::data.add_key(sc);
    } else {
      str_= get_map().find(sc)->second;
    }    
  }

  explicit KeyBase(unsigned int i): str_(i) {
    //IMP_assert(get_rmap().size() > i, "There is no such attribute " << i);
  }

  //! Turn a key into a pretty string
  const std::string get_string() const {
    if (is_default()) return std::string("NULL");
    return get_string(str_);
    //return str_;
  }

  IMP_COMPARISONS_1(str_)

  void show(std::ostream &out = std::cout) const {
    out << "\"" << get_string() << "\"";
  }

  unsigned int get_index() const {
    IMP_assert(!is_default(),
               "Cannot get index on defaultly constructed Key");
    return str_;
  }

  //! Show all the keys of this type
  static void show_all(std::ostream &out);

  //! Get a list of all of the keys of this type
  /**
     This can be used to check for typos and similar keys.
   */
  static std::vector<std::string> get_all_strings();

  //! Get the total number of keys of this type
  /**
     This is mostly for debugging to makes sure that there are no extra
     keys created.
   */
  static unsigned int get_number_unique() {
    return get_map().size();
  }

#ifndef SWIG
  /** \todo These should be protected, I'll try to work how
   */
  This operator++() {
    ++str_;
    return *this;
  }
  This operator--() {
    --str_;
    return *this;
  }
  This operator+(int o) const {
    This c=*this;
    c.str_+= o;
    return c;
  }
#endif
};

IMP_OUTPUT_OPERATOR_1(KeyBase)


template <class T>
inline const internal::KeyData::Map& KeyBase<T>::get_map()
{
  return internal::KeyDataHolder<T>::data.get_map();
}

template <class T>
inline const internal::KeyData::RMap& KeyBase<T>::get_rmap()
{
  return internal::KeyDataHolder<T>::data.get_rmap();
}

template <class T>
inline bool KeyBase<T>::is_default() const
{
  return str_==-1;
}

template <class T>
inline const std::string &KeyBase<T>::get_string(int i)
{
  IMP_assert(static_cast<unsigned int>(i)
             < get_rmap().size(),
             "Corrupted "  << " Key " << i
             << " vs " << get_rmap().size());
  return get_rmap()[i];
}


template <class T>
inline void KeyBase<T>::show_all(std::ostream &out)
{
  internal::KeyDataHolder<T>::data.show(out);
}

template <class T>
std::vector<std::string> KeyBase<T>::get_all_strings()
{
  std::vector<std::string> str;
  for (internal::KeyData::Map::const_iterator it= get_map().begin();
       it != get_map().end(); ++it) {
    str.push_back(it->first);
  }
  return str;
}

} // namespace IMP

#endif  /* __IMP_KEY_H */
