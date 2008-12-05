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
#include "internal/ObjectContainer.h"
#include "macros.h"

/** \internal
 */
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
Ucname##Iterator lcname##s_begin() {return lcname##_vector_.begin();}  \
Ucname##Iterator lcname##s_end() {return lcname##_vector_.end();}      \
Ucname##ConstIterator lcname##s_begin() const {                        \
return lcname##_vector_.begin();}                                    \
Ucname##ConstIterator lcname##s_end() const {                          \
return lcname##_vector_.end();}                                      \
private:                                                                \
/** \internal */                                                       \
Container lcname##_vector_;                                            \
protection:

/** \internal
 */
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
/** \short Clear the contents of the container */                     \
void Class::clear_##lcname##s(){                                      \
lcname##_vector_.clear();                                           \
Onchanged;                                                          \
}                                                                     \


//! Use this to add a container of IMP objects
/**
 Such a container adds public methods add_foo, get_foo, get_number_of_foo
 and a private type foo_iterator, with methods foo_begin, foo_end.
 \param[in] protection The level of protection for the container.
 \param[in] Ucname The name of the type in uppercase
 \param[in] lcname The name of the type in lower case
 \param[in] Data The type of the data to store.

 \note the type Ucnames must be declared and be a vector of
 Data.
 */
#define IMP_LIST(protection, Ucname, lcname, Data)                      \
protection:                                                           \
/** \short Remove any occurences of d from the container */           \
void erase_##lcname(Data d);                                          \
/** \short Get a container of all the objects.
This is for Python as the container can be used like a Python list*/\
const Ucname##s &get_##lcname##s() const {                             \
return static_cast< const Ucname##s &>(lcname##_vector_);           \
}                                                                     \
void set_##lcname##s(const Ucname##s &ps) {                           \
clear_##lcname##s();                                                \
add_##lcname##s(ps);                                                \
}                                                                     \
IMP_CONTAINER_CORE(protection, Ucname, lcname, Data, unsigned int,    \
IMP::internal::Vector<Data>)



//! Use this to add a container of IMP objects
/**
 This code should go in a .cpp file. One macro for each IMP_CONTAINER.
 \param[in] init Code to modify the passed in object. The object is obj
 its index index.
 \param[in] OnChanged Code to get executed when the container changes.
 */
#define IMP_LIST_IMPL(Class, Ucname, lcname, Data, init, OnChanged)     \
IMP_CONTAINER_CORE_IMPL(Class, Ucname, lcname, Data, unsigned int,    \
init, OnChanged)                              \
/** \short Remove any occurences of d from the container */           \
void Class::erase_##lcname(Data d) {                                  \
for (Ucname##Iterator it= lcname##s_begin();                        \
it != lcname##s_end(); ++it) {                                 \
if (*it == d) {                                                   \
lcname##_vector_.erase(it); break;                              \
}                                                                 \
}                                                                   \
OnChanged;                                                          \
}                                                                     \



//! Use this to add a set of IMP objects owned by the containing one
/**
 Such a container adds public methods add_foo, get_foo, get_number_of_foo
 and a private type foo_iterator, with methods foo_begin, foo_end.
 \param[in] Ucname The name of the type in uppercase
 \param[in] lcname The name of the type in lower case
 \param[in] IndexType The type to use for the index. This should be
 an instantiation of Index<T> or something similar.

 \note The type Ucnames must be declared and be a vector of
 Data.
 \note these containers are always public
 */
#define IMP_CONTAINER(Ucname, lcname, IndexType)            \
public:                                                   \
void remove_##lcname(IndexType i) ;                       \
private:                                                                \
/** \internal
This is an implementation detail.*/                                 \
typedef IMP::internal::ObjectContainer<Ucname, IndexType>               \
Ucname##Container;                                                      \
IMP_CONTAINER_CORE(public, Ucname, lcname, Ucname*, IndexType,          \
Ucname##Container)



//! Use this to add a container of IMP objects
/**
 This code should go in a .cpp file. One macro for each
 IMP_CONTAINER.
 \param[in] init Code to modify the passed in object. The object is obj
 its index index.
 \param[in] onchanged Code to execute when the container is changed.
 \param[in] onremove Code to execute when an object is removed. The object
 being removed is obj.
 */
#define IMP_CONTAINER_IMPL(Class, Ucname, lcname, IndexType, init,      \
onchanged, onremove)                                                    \
void Class::remove_##lcname(IndexType i) {                            \
Ucname* obj= lcname##_vector_[i];                                   \
onremove;                                                           \
lcname##_vector_.remove(i);                                         \
if (false) std::cout << *obj;                                       \
}                                                                     \
IMP_CONTAINER_CORE_IMPL(Class, Ucname, lcname, Ucname*, IndexType,    \
init,onchanged)

#endif  /* IMP_INTERNAL_CONTAINER_MACROS_H */
