/**
 *  \file IMP/Key.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_KEY_H
#define IMPKERNEL_KEY_H

#include "utility.h"
#include "internal/key_helpers.h"
#include <IMP/check_macros.h>
#include <IMP/comparison_macros.h>
#include <IMP/hash_macros.h>
#include <IMP/log_macros.h>
#include <IMP/thread_macros.h>
#include <IMP/Value.h>
#include <vector>

IMPKERNEL_BEGIN_NAMESPACE

//! A base class for Keys
/** This class does internal caching of the strings to accelerate the
    name lookup. It is better to create a Key and reuse it
    rather than recreate it many times from strings.

    If you use this with a new type, you must add a new definition of
    attribute_table_index. Yes, this is an evil hack, but I couldn't
    get linking to work with static members of the template class.

    The keys in \imp maintain a cached mapping between strings and indexes.
    This mapping is global--that is all \imp Models and Particles in the
    same program use the same mapping for each type of key. The type of
    the key is determined by an integer which should be unique for
    each type. If the integer is not unique, everything works, just
    more memory is wasted and types are interconvertible.

    Keys used for storing attributes in particles should never be statically
    initialized. While this is annoying, statically initializing them is bad,
    as unused attribute keys can result in wasted memory in each particle.
 */
template <unsigned int ID>
class Key : public Value {
 private:

  //! returns a static structure with mapping between
  //! key int identifiers and strings
  static internal::KeyData& get_key_data() {
#ifndef IMPKERNEL_INTERNAL_OLD_COMPILER
    static internal::KeyData static_key_data_(ID);
    return static_key_data_;
#else
    return IMP::internal::get_key_data(ID);
#endif
  }

 private:
  int str_;
  static const internal::KeyData::Map& get_map() {
    return get_key_data().get_map();
  }
  static const internal::KeyData::RMap& get_rmap() {
    IMP::internal::KeyData const& kd=get_key_data();
    IMP::internal::KeyData::RMap const& ret=kd.get_rmap();
    return ret;
  }

  //! returns the index of sc, adds it if it's not there
  static unsigned int find_or_add_index(std::string const& sc) {
    IMP_USAGE_CHECK(!sc.empty(), "Can't create a key with an empty name");
    unsigned int val;
    IMP_OMP_PRAGMA(critical(imp_key)) {
      if (get_map().find(sc) == get_map().end()) {
        val = get_key_data().add_key(sc);
      } else {
        val = get_map().find(sc)->second;
      }
    }
    return val;
  }


  static unsigned int find_index(std::string const& sc) {
    IMP_USAGE_CHECK(!sc.empty(), "Can't create a key with an empty name");
    unsigned int val;
    IMP_OMP_PRAGMA(critical(imp_key)) {
      IMP_USAGE_CHECK( get_key_exists(sc), "Key<" << ID << ">::find_index():"
		       << " You must explicitly create the type first: "
		       << sc);
      val = get_map().find(sc)->second;
    }
    return val;
  }

 private:
  bool is_default() const;

 public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  static unsigned int get_ID() { return ID; }

  static const std::string get_string(int i) {
    std::string val;
    IMP_OMP_PRAGMA(critical(imp_key)) {
      if (static_cast<unsigned int>(i) < get_rmap().size()) {
        val = get_rmap()[i];
      }
    }
    if (val.empty()) {
      IMP_FAILURE("Corrupted Key Table asking for key "
                  << i << " with a table of size " << get_rmap().size());
    }
    return val;
  }

#endif
  //! make a default key in a well-defined null state
  Key() : str_(-1) {}

    //! Generate a key object from the given string
    /**
       Generate a key object from the given string. 
       
       @param c key string representation
       @param is_implicit_add_permitted If true, a key for c can be created even if it hasn't
                        been created earlier. If false, than it is assumed that a key for c 
                        has already been instantiated by e.g., a previous
			call to Key(c, true) or using Key::add_key().
			Formally, it is assumed that get_has_key(c) is true.
    
       @note This operation can be expensive, so please cache the result.
    */
    explicit Key(std::string const& c, bool is_implicit_add_permitted=true) 
      : str_(is_implicit_add_permitted ? find_or_add_index(c) : find_index(c))
      {}

#if !defined(IMP_DOXYGEN)
      //! this is a fast and lean constructor that should be used
      //! whenever performence is of the essence
      explicit Key(unsigned int i) : str_(i) {
	IMP_INTERNAL_CHECK(str_ >= 0, "Invalid initializer " << i);
	// cannot check here as we need a past end iterator
      }
#endif

  static unsigned int add_key(std::string sc) {
    IMP_USAGE_CHECK(!sc.empty(), "Can't create a key with an empty name");
    unsigned int val;
    IMP_OMP_PRAGMA(critical(imp_key))
    IMP_LOG_PROGRESS("Key::add_key " << sc  << " ID " << ID << std::endl);
    val = get_key_data().add_key(sc);
    return val;
  }

  //! Return true if there already is a key with that string
  static bool get_key_exists(std::string sc) {
    bool val;
    IMP_OMP_PRAGMA(critical(imp_key))
    val = get_map().find(sc) != get_map().end();
    return val;
  }

  //! Turn a key into a pretty string
  const std::string get_string() const {
    if (is_default()) return std::string("nullptr");
    std::string val;
    val = get_string(str_);
    return val;
  }

  IMP_COMPARISONS_1(Key, str_);

  IMP_HASHABLE_INLINE(Key, return str_;)

  IMP_SHOWABLE_INLINE(Key, out << "\"" << get_string() << "\"";);

  //! Make new_name an alias for old_key
  /** Afterwards
      \code
      Key<ID>(old_key.get_string()) == Key<ID>(new_name)
      \endcode
   */
  static Key<ID> add_alias(Key<ID> old_key,
			   std::string new_name) {
    IMP_INTERNAL_CHECK( get_map().find(new_name) == get_map().end(),
			"The name is already taken with an existing key or alias");
    get_key_data().add_alias(new_name, old_key.get_index());
    return Key<ID>(new_name.c_str());
  }

  static unsigned int get_number_of_keys() {
    return get_rmap().size();
  }

#ifndef DOXYGEN
  unsigned int get_index() const {
    IMP_INTERNAL_CHECK(!is_default(),
                       "Cannot get index on defaultly constructed Key");
    return str_;
  }
#endif

  //! Show all the keys of this type
  static void show_all(std::ostream& out);

  //! Get a list of all of the keys of this type
  /**
     This can be used to check for typos and similar keys.
   */
  static Vector<std::string> get_all_strings();

  //! Get the total number of keys of this type
  /**
     This is mostly for debugging to make sure that there are no extra
     keys created.
   */
  static unsigned int get_number_unique() { return get_rmap().size(); }

#ifndef SWIG
  /** \todo These should be protected, I'll try to work how
   */
  Key& operator++() {
    ++str_;
    return *this;
  }
  Key& operator--() {
    --str_;
    return *this;
  }
  Key operator+(int o) const {
    Key c = *this;
    c.str_ += o;
    return c;
  }
#endif
};

#ifndef IMP_DOXYGEN


template <unsigned int ID>
inline std::ostream& operator<<(std::ostream& out, Key<ID> k) {
  k.show(out);
  return out;
}

template <unsigned int ID>
inline bool Key<ID>::is_default() const {
  return str_ == -1;
}

template <unsigned int ID>
  inline void Key<ID>::show_all(std::ostream& out) {
  IMP_OMP_PRAGMA(critical(imp_key))
    get_key_data().show(out);
}

template <unsigned int ID>
Vector<std::string> Key<ID>::get_all_strings() {
  Vector<std::string> str;
  IMP_OMP_PRAGMA(critical(imp_key))
  for (internal::KeyData::Map::const_iterator it = get_map().begin();
       it != get_map().end(); ++it) {
    str.push_back(it->first);
  }
  return str;
}
#endif

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_KEY_H */

