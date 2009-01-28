/*
 *  \file container_macros.h
 *  \brief Macros to define containers of objects
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_CONTAINER_MACROS_H
#define IMP_INTERNAL_CONTAINER_MACROS_H

#include "internal/Vector.h"
#include "macros.h"

#define IMP_CONTAINER_CORE(protection, Ucname, lcname, Data, IndexType,\
Container)                                  \
protection:                                                          \
/** \short Add an object.
\param[in] obj Pointer to the object
\return index of object within the object
*/                                                                    \
IndexType add_##lcname(Data obj);                                     \
/** \short Add several objects.
\param[in] obj a vector of pointers
*/                                                                    \
void add_##lcname##s(const std::vector<Data>& obj);                   \
/** \short Clear the contents of the container */                     \
void clear_##lcname##s();                                             \
/** \short return the number of objects*/                             \
unsigned int get_number_of_##lcname##s() const {                          \
return lcname##_vector_.size();}                                    \
/** \short return if there are any objects*/                             \
unsigned int get_has_##lcname##s() const {                          \
return !lcname##_vector_.empty();}                                    \
/** \short Get object refered to by the index
\throws IndexException if the index is out of range
*/                                                                   \
Data get_##lcname(IndexType i) const {                                \
return lcname##_vector_[i];                                         \
}                                                                     \
/** \short An iterator through the objects.
The value type is a pointer.*/                                     \
typedef Container::iterator Ucname##Iterator;                          \
/** \short A const iterator through the objects.
The value type is a pointer.*/                                     \
typedef Container::const_iterator Ucname##ConstIterator;               \
/** Begin iterating through container */                               \
Ucname##Iterator lcname##s_begin() {return lcname##_vector_.begin();}  \
/** End iterating through container */                               \
Ucname##Iterator lcname##s_end() {return lcname##_vector_.end();}      \
/** Begin iterating through container */                               \
Ucname##ConstIterator lcname##s_begin() const {                        \
return lcname##_vector_.begin();}                                    \
/** End iterating through container */                               \
Ucname##ConstIterator lcname##s_end() const {                          \
return lcname##_vector_.end();}                                      \
private:                                                                \
Container lcname##_vector_;                                            \
protection:


#define IMP_CONTAINER_CORE_IMPL(Class, Ucname, lcname, Data, IndexType, \
Init_obj, Onchanged)                    \
IndexType Class::add_##lcname(Data obj) {                             \
IndexType index= lcname##_vector_.push_back(obj);                   \
Init_obj;                                                           \
Onchanged;                                                          \
if (false) {index=index; obj=obj;};                                 \
return index;                                                       \
}                                                                     \
void Class::add_##lcname##s(const std::vector<Data> &objs) {          \
unsigned int osz= lcname##_vector_.size();                          \
lcname##_vector_.insert(lcname##_vector_.end(), objs.begin(),       \
objs.end());                                \
for (unsigned int i=0; i< objs.size(); ++i) {                       \
Data obj= lcname##_vector_[osz+i];                                \
IndexType index(osz+i);                                           \
Init_obj;                                                         \
if (false) {obj=obj; index=index;}                                \
}                                                                   \
Onchanged;                                                          \
}                                                                     \
void Class::clear_##lcname##s(){                                      \
lcname##_vector_.clear();                                           \
Onchanged;                                                          \
}                                                                     \


//! Use this to add a list of objects to a class.
/** The difference between a IMP_LIST and IMP_CONTAINER is that an
 IMP_CONTAINER uses Index objects to return access to the objects and this
 container just uses ints.

 Such a container adds public methods add_foo, get_foo, get_number_of_foo
 and a private type foo_iterator, with methods foo_begin, foo_end.
 \param[in] protection The level of protection for the container
 (public, private).
 \param[in] Ucname The name of the type of container in uppercase.
 \param[in] lcname The name of the type of container in lower case.
 \param[in] Data The type of the data to store.

 \note the type Ucnames must be declared and be a vector of
 Data.
 */
#define IMP_LIST(protection, Ucname, lcname, Data)                      \
protection:                                                           \
/** @name Methods acting on a contained list*/                        \
/*@{*/                                                                \
/** \short Remove any occurences of d from the container */           \
void remove_##lcname(Data d);                                          \
void set_##lcname##s(const Ucname##s &ps) {                           \
clear_##lcname##s();                                                \
add_##lcname##s(ps);                                                \
}                                                                     \
IMP_CONTAINER_CORE(protection, Ucname, lcname, Data, unsigned int,    \
IMP::internal::Vector<Data>)                                          \
/*@}*/                                                                \




//! This should go in a .cpp file for the respective class.
/**
 This code should go in a .cpp file. One macro for each IMP_CONTAINER.
 \param[in] The name of the class containing this container.
 \param[in] protection The level of protection for the container
 (public, private).
 \param[in] Ucname The name of the type of container in uppercase.
 \param[in] lcname The name of the type of container in lower case.
 \param[in] Data The type of the data to store.
 \param[in] OnAdd Code to modify the passed in object. The object is obj
 its index index.
 \param[in] OnChanged Code to get executed when the container changes.
 \param[in] OnRemove Code to get executed when the an object is removed.
 */
#define IMP_LIST_IMPL(Class, Ucname, lcname, Data, OnAdd,             \
                      OnChanged, OnRemoved)                           \
  IMP_CONTAINER_CORE_IMPL(Class, Ucname, lcname, Data, unsigned int,  \
                          OnAdd, OnChanged)                            \
  void Class::remove_##lcname(Data d) {                                \
    for (Ucname##Iterator it= lcname##s_begin();                       \
         it != lcname##s_end(); ++it) {                                \
      if (*it == d) {                                                  \
        Data obj=*it;                                                  \
        OnRemoved;                                                     \
        if (0) std::cout << obj;                                       \
        lcname##_vector_.erase(it); break;                             \
      }                                                                \
    }                                                                  \
    OnChanged;                                                         \
  }                                                                    \

#endif  /* IMP_INTERNAL_CONTAINER_MACROS_H */
