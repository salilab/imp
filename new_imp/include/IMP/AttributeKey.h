/**
 *  \file AttributeKey.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_ATTRIBUTE_KEY_H
#define __IMP_ATTRIBUTE_KEY_H

#include "utility.h"
#include "log.h"
#include <map>


namespace IMP
{
template <class TT>
static std::string type_string()
{
  return "Unknown";
}
template <>
static std::string type_string<Float>()
{
  return "Float";
}
template <>
static std::string type_string<String>()
{
  return "String";
}
template <>
static std::string type_string<Int>()
{
  return "Int";
}





//! A base class for AttributeKeys
/** This class does internal caching of the strings to accelerate the
    name lookup. It is better to create an AttributeKey and reuse it
    rather than recreate it many times from strings.
 */
template <class T>
class AttributeKey
{

  std::string str_;

  // Hack to avoid having to manually allocate memory for the table
  /*Map &map() const {
    static Map map;
    return map;
  }
  RMap &rmap() const {
    static RMap rmap;
    return rmap;
    }*/
  static const std::string& default_string() {
    const static std::string nil="NULL";
    return nil;
  }
  bool is_default() const {
    return str_.empty();
  }
public:
  typedef AttributeKey<T> This;

  //! make a default (uninitalized) key
  AttributeKey():str_() {}
  //! Generate a key from the given string
  AttributeKey(const char *c) {
    /*if (map_.find(c) == map_.end()) {

      int sz= map_.size();
      map_[c]=sz;
      rmap_.push_back(c);
      str_= sz;
      std::cerr << "Creating " << type_string<T>() << " attribute \"" << c
                << "\" with id " << sz
                << " string is " << get_string() << std::endl;
    } else {
      str_= map_.find(c)->second;
      }*/
    str_=c;
  };

  /*AttributeKey(const AttributeKey<T> &o): str_(o.str_) {
    IMP_assert(is_default()
               || static_cast<unsigned int>(str_) < rmap_.size(),
               "AttributeKey issue. Key is "
               << str_ << " but there are only " << rmap_.size()
               << " in map");
               }*/

  //! Turn a key into a pretty string
  const std::string &get_string() const {
    if (is_default()) return default_string();
    /*IMP_assert(static_cast<unsigned int>(str_)
               < rmap_.size(),
               "Corrupted " << type_string<T>() << " AttributeKey " << str_
               << " vs " << rmap_.size());
               return rmap_[str_];*/
    return str_;
  }

  IMP_COMPARISONS_1(str_)

  std::ostream &show(std::ostream &out) const {
    return out << "\"" << get_string() << "\"";
  }

  /*static void list () {
    IMP_assert(map_.size() == rmap_.size(),
                     "Something is up with the maps.");
    std::cerr << "Dumping keys for " << type_string<T>()
              << " attributes (" << rmap_.size() << ")\n";
    for (unsigned int i=0; i< rmap_.size(); ++i) {
      std::cerr << i << "\"" << rmap_[i] << "\"" << std::endl;
    }
    }*/
};


/*template <class T>
std::map<std::string, int> AttributeKey<T>::map_;
template <class T>
std::vector<std::string> AttributeKey<T>::rmap_;*/


IMP_OUTPUT_OPERATOR_1(AttributeKey)
}

#endif  /* __IMP_ATTRIBUTE_KEY_H */
