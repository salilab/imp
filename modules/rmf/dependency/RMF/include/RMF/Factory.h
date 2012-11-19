/**
 *  \file RMF/Factory.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF__FACTORY_H
#define RMF__FACTORY_H

#include <RMF/config.h>
namespace RMF {

/** Factories in RMF allow one to create Decorator objects as well
    as check whether a given node has the values needed for a particular
    decorator type in a given frame.
 */
template <class DecoratorType>
class Factory {
  /* It is important that this not save the FileHandle so as
     not to keep the file open.*/
 protected:
  Factory(){}
  template <class Key, class Node>
  bool get_has_value(Node n, const Key &key) const {
    return n.get_has_value(key);
  }
  template <class Keys, class Node>
  bool get_has_values(Node n, const Keys &keys) const {
    return n.get_has_value(keys[0]);
  }
  template <class TypeTraits>
  Key<TypeTraits>
  get_key(FileConstHandle fh, Category cat, const char* name) const {
    return fh.get_key<TypeTraits>(cat, name);
  }
  template <class TypeTraits>
  vector<Key<TypeTraits> >
  get_keys(FileConstHandle fh, Category cat, const Strings &names) const {
    return fh.get_keys<TypeTraits>(cat, names);
  }
 public:
  typedef DecoratorType Decorator;
  RMF_SHOWABLE(Factory,
                   Decorator::get_decorator_type_name()
                   << "Factory");
};

} /* namespace RMF */

#endif /* RMF__FACTORY_H */
