/**
 *  \file Key.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_KEY_H
#define IMP_KEY_H

#include "macros.h"
#include "exception.h"
#include "utility.h"
#include "internal/key_helpers.h"

#include <map>
#include <vector>

IMP_BEGIN_NAMESPACE

//! A base class for  Keys
/** This class does internal caching of the strings to accelerate the
    name lookup. It is better to create an Key and reuse it
    rather than recreate it many times from strings.

    If you use this with a new type, you must add a new definition of
    attribute_table_index. Yes, this is an evil hack, but I couldn't
    get linking to work with static members of the template class.

    The keys in IMP maintain a cached mapping between strings and indexes.
    This mapping is global--that is all IMP Models and Particles in the
    same program use the same mapping for each type of key. The type of
    the key is determined by an integer which should be unique for
    each type. If the integer is not unique, everything works, just
    more memory is wasted and types are interconvertible

    Keys used for storing attributes in particles should never be statically
    initialized. While this is annoying, statically initializing them is bad,
    as unused attribute keys can result in wasted memory in each particle.

    If LazyAdd is true, keys created with a new string will be added,
    otherwise this is an error.

    \note Keys objects are ordered.
 */
template <unsigned int ID, bool LazyAdd>
class KeyBase: public NullDefault, public Comparable
{
  int str_;

  static const internal::KeyData::Map& get_map()
  {
    return IMP::internal::get_key_data(ID).get_map();
  }
  static const internal::KeyData::RMap& get_rmap() {
    return IMP::internal::get_key_data(ID).get_rmap();
  }


  static unsigned int find_index(std::string sc) {
    if (get_map().find(sc) == get_map().end()) {
      IMP_INTERNAL_CHECK(LazyAdd, "You must explicitly create the type"
                 << " first: " << sc);
      return IMP::internal::get_key_data(ID).add_key(sc);
    } else {
      return get_map().find(sc)->second;
    }
  }
private:
 bool is_default() const;
public:
#ifndef DOXYGEN
  static unsigned int get_ID() {
    return ID;
  }

  static const std::string get_string(int i)
  {
    if (static_cast<unsigned int>(i)
        < get_rmap().size()) {
      return get_rmap()[i];
    } else {
      std::cerr << "Corrupted Key Table asking for key " << i
                << " with a table of size " << get_rmap().size()
                << std::endl;
      std::ostringstream oss;
      oss<< "Invalid index " << i;
      return oss.str();
    }
  }

  typedef KeyBase<ID, LazyAdd> This;
#endif

  //! make a default key in a well-defined null state
  KeyBase(): str_(-1) {}

  //! Generate a key from the given string
  /** This operation can be expensive, so please cache the result.*/
  explicit KeyBase(std::string c) {
    str_= find_index(c);
  }


#if !defined(IMP_DOXYGEN)
  explicit KeyBase(unsigned int i): str_(i) {
    IMP_INTERNAL_CHECK(str_ >= 0, "Invalid initializer " << i);
    // cannot check here as we need a past end iterator
  }
#endif

  static unsigned int add_key(std::string sc) {
    return IMP::internal::get_key_data(ID).add_key(sc);
  }

  //! Return true if there already is a key with that string
  static bool get_key_exists(std::string sc) {
    return get_map().find(sc) != get_map().end();
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

  //! Make new_name an alias for old_key
  /** Afterwards
      \code
      KeyBase<ID>(old_key.get_string()) == KeyBase<ID>(new_name)
      \endcode
   */
  static KeyBase<ID, LazyAdd> add_alias(KeyBase<ID, LazyAdd> old_key,
                                        std::string new_name) {
    IMP_INTERNAL_CHECK(get_map().find(new_name) == get_map().end(),
               "The name is already taken with an existing key or alias");
    IMP::internal::get_key_data(ID).add_alias(new_name, old_key.get_index());
    return KeyBase<ID, LazyAdd>(new_name.c_str());
  }

#ifndef DOXYGEN
  unsigned int get_index() const {
    IMP_INTERNAL_CHECK(!is_default(),
               "Cannot get index on defaultly constructed Key");
    return str_;
  }
#endif

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
  This& operator++() {
    ++str_;
    return *this;
  }
  This& operator--() {
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


template <unsigned int ID, bool LA>
std::ostream &operator<<(std::ostream &out, KeyBase<ID, LA> k) {
  k.show(out);
  return out;
}

template <unsigned int ID, bool LA>
inline bool KeyBase<ID, LA>::is_default() const
{
  return str_==-1;
}


template <unsigned int ID, bool LA>
inline void KeyBase<ID, LA>::show_all(std::ostream &out)
{
  internal::get_key_data(ID).show(out);
}

template <unsigned int ID, bool LA>
std::vector<std::string> KeyBase<ID, LA>::get_all_strings()
{
  std::vector<std::string> str;
  for (internal::KeyData::Map::const_iterator it= get_map().begin();
       it != get_map().end(); ++it) {
    str.push_back(it->first);
  }
  return str;
}

IMP_END_NAMESPACE

#endif  /* IMP_KEY_H */
