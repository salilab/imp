/**
 *  \file Key.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_KEY_H
#define __IMP_KEY_H

#include "Base_Types.h"
#include "utility.h"
#include "log.h"
#include <map>
#include <vector>


namespace IMP
{


namespace internal {


struct KeyData {
  std::map<std::string, int> map;
  std::vector<std::string> rmap;
};

IMPDLLEXPORT extern unsigned int next_attribute_table_index_;


// print a list of attributes used so far by the program
void IMPDLLEXPORT show_attributes(std::ostream &out);

extern IMPDLLEXPORT std::vector<KeyData> attribute_key_data;

}









//! A base class for  Keys
/** This class does internal caching of the strings to accelerate the
    name lookup. It is better to create an Key and reuse it
    rather than recreate it many times from strings.

    If you use this with a new type, you must add a new definition of
    attribute_table_index. Yes, this is an evil hack, but I couldn't
    get linking to work with static members of the template class.
 */
template <class T>
class Key
{
  int str_;

  typedef T Type;

  static internal::KeyData& data();

  bool is_default() const;

public:
  static const std::string &get_string(int i) ;

  typedef Key<T> This;

  //! make a default (uninitalized) key
  Key():str_(-1) {}


  //! Generate a key from the given string
  Key(const char *c) {
    std::string sc(c);
    if (data().map.find(sc) == data().map.end()) {

      int sz= data().map.size();
      data().map[sc]=sz;
      data().rmap.push_back(sc);
      str_= sz;
      IMP_assert(data().rmap.size() == data().map.size(), "Unequal map sizes")
    } else {
      str_= data().map.find(sc)->second;
    }
    //str_=c;
  };

  explicit Key(unsigned int i): str_(i) {
    IMP_assert(data().rmap.size() > i, "There is no such attribute " << i)
  }

  //! Turn a key into a pretty string
  const std::string get_string() const {
    if (is_default()) return std::string("NULL");
    return get_string(str_);
    //return str_;
  }

  IMP_COMPARISONS_1(str_)

  std::ostream &show(std::ostream &out) const {
    return out << "\"" << get_string() << "\"";
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
  static std::vector<String> get_all_strings();

  //! Get the total number of keys of this type
  /**
     This is mostly for debugging to makes sure that there are no extra
     keys created.
   */
  static unsigned int get_number_unique() {
    return data().map.size();
  }
};

IMP_OUTPUT_OPERATOR_1(Key)








template <class T>
inline internal::KeyData& Key<T>::data()
{
  unsigned int i= internal::attribute_table_index(T());
  if ( internal::attribute_key_data.size() <= i) {
    internal::attribute_key_data.resize(i+1);
  }
  return internal::attribute_key_data[i];
}

template <class T>
inline bool Key<T>::is_default() const
{
  return str_==-1;
}

template <class T>
inline const std::string &Key<T>::get_string(int i)
{
  IMP_assert(static_cast<unsigned int>(i)
             < data().rmap.size(),
             "Corrupted "  << " Key " << i
             << " vs " << data().rmap.size());
  return data().rmap[i];
}


template <class T>
inline void Key<T>::show_all(std::ostream &out)
{
  const internal::KeyData &d= data();
  for (std::map<std::string, int>::const_iterator it= d.map.begin();
       it != d.map.end(); ++it) {
    out << "\"" << it->first << "\" ";
  }
}

template <class T>
std::vector<String> Key<T>::get_all_strings()
{
  std::vector<String> str;
  const internal::KeyData &d= data();
  for (std::map<std::string, int>::const_iterator it= d.map.begin();
       it != d.map.end(); ++it) {
    str.push_back(it->first);
  }
  return str;
}

}

#endif  /* __IMP_KEY_H */
