/**
 *  \file IMP/macros.h    \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_MACROS_H
#define IMP_MACROS_H

#ifdef IMP_DOXYGEN
//! Hide something from doxygen
#define IMP_NO_DOXYGEN(x)
//! Only show something to doxygen
#define IMP_ONLY_DOXYGEN(x) x
//! Show the first to DOXYGEN and the second to the world
#define IMP_SWITCH_DOXYGEN(doxygen, nodoxygen) doxygen
#else
#define IMP_NO_DOXYGEN(x) x
#define IMP_ONLY_DOXYGEN(x)
#define IMP_SWITCH_DOXYGEN(doxygen, nodoxygen) nodoxygen
#endif

#if defined(SWIG)
#define IMP_NO_SWIG(x)
#else
//! Hide the line when SWIG is compiled or parses it
#define IMP_NO_SWIG(x) x
#endif

#ifdef IMP_DOXYGEN
//! Implement comparison in a class using a compare function
/** The macro requires that This be defined as the type of the current class.
    The compare function should take a const This & and return -1, 0, 1 as
    appropriate.
 */
#define IMP_COMPARISONS
#else
#define IMP_COMPARISONS                                                 \
  bool operator==(const This &o) const {                                \
    return (compare(o) == 0);                                           \
  }                                                                     \
  bool operator!=(const This &o) const {                                \
    return (compare(o) != 0);                                           \
  }                                                                     \
  bool operator<(const This &o) const {                                 \
    return (compare(o) <0);                                             \
  }                                                                     \
  bool operator>(const This &o) const {                                 \
    return (compare(o) > 0);                                            \
  }                                                                     \
  bool operator>=(const This &o) const {                                \
    return !(compare(o) < 0);                                           \
  }                                                                     \
  bool operator<=(const This &o) const {                                \
    return !(compare(o) > 0);                                           \
  }                                                                     \
  IMP_NO_SWIG(template <class T> friend int compare(const T&a, const T&b);
#endif

#ifdef IMP_DOXYGEN                                                         \
//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_1(field)
#else
#define IMP_COMPARISONS_1(field)                                        \
  bool operator==(const This &o) const {                                \
    return (field== o.field);                                           \
  }                                                                     \
  bool operator!=(const This &o) const {                                \
    return (field!= o.field);                                           \
  }                                                                     \
  bool operator<(const This &o) const {                                 \
    return (field< o.field);                                            \
  }                                                                     \
  bool operator>(const This &o) const {                                 \
    return (field> o.field);                                            \
  }                                                                     \
  bool operator>=(const This &o) const {                                \
    return (field>= o.field);                                           \
  }                                                                     \
  bool operator<=(const This &o) const {                                \
    return (field<= o.field);                                           \
  }                                                                     \
  int compare(const This &o) const {                                    \
    if (operator<(o)) return -1;                                        \
    else if (operator>(o)) return 1;                                    \
    else return 0;                                                      \
  }

#endif

#ifdef IMP_DOXYGEN
//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_2(f0, f1)
#else
#define IMP_COMPARISONS_2(f0, f1)                                       \
  bool operator==(const This &o) const {                                \
    return (f0== o.f0 && f1==o.f1);                                     \
  }                                                                     \
  bool operator!=(const This &o) const {                                \
    return (f0!= o.f0 || f1 != o.f1);                                   \
  }                                                                     \
  bool operator<(const This &o) const {                                 \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    else return f1 < o.f1;                                              \
  }                                                                     \
  bool operator>(const This &o) const {                                 \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    else return f1 > o.f1;                                              \
  }                                                                     \
  bool operator>=(const This &o) const {                                \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  bool operator<=(const This &o) const {                                \
    return operator<(o) || operator==(o);                               \
  }                                                                     \
  int compare(const This &o) const {                                    \
    if (operator<(o)) return -1;                                        \
    else if (operator>(o)) return 1;                                    \
    else return 0;                                                      \
  }
#endif


#ifdef IMP_DOXYGEN
//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_3(f0, f1, f2)
#else
#define IMP_COMPARISONS_3(f0, f1, f2)                                   \
  bool operator==(const This &o) const {                                \
    return (f0== o.f0 && f1==o.f1 && f2 == o.f2);                       \
  }                                                                     \
  bool operator!=(const This &o) const {                                \
    return (f0!= o.f0 || f1 != o.f1 || f2 != o.f2);                     \
  }                                                                     \
  bool operator<(const This &o) const {                                 \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    if (f1< o.f1) return true;                                          \
    else if (f1 > o.f1) return false;                                   \
    else return f2 < o.f2;                                              \
  }                                                                     \
  bool operator>(const This &o) const {                                 \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    if (f1 > o.f1) return true;                                         \
    else if (f1 < o.f1) return false;                                   \
    else return f2 > o.f2;                                              \
  }                                                                     \
  bool operator>=(const This &o) const {                                \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  bool operator<=(const This &o) const {                                \
    return operator<(o) || operator==(o);                               \
  }                                                                     \
  int compare(const This &o) const {                                    \
    if (operator<(o)) return -1;                                        \
    else if (operator>(o)) return 1;                                    \
    else return 0;                                                      \
  }
#endif

#ifdef IMP_DOXYGEN                                                         \
//! Implement operator<< on class name, assuming it has one template argument
/** \copydetails IMP_OUTPUT_OPERATOR
 */
#define IMP_OUTPUT_OPERATOR_1(name)
#else
#define IMP_OUTPUT_OPERATOR_1(name)                                     \
  template <class L>                                                    \
  inline std::ostream& operator<<(std::ostream &out, const name<L> &i)  \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }
#endif

#ifdef IMP_DOXYGEN
//! Implement operator<< on class name, assuming it has two template arguments
/** \copydetails IMP_OUTPUT_OPERATOR
 */
#define IMP_OUTPUT_OPERATOR_2(name)
#else
#define IMP_OUTPUT_OPERATOR_2(name)                                     \
  template <class L, class M>                                           \
  inline std::ostream& operator<<(std::ostream &out, const name<L, M> &i) \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }
#endif

#ifdef IMP_DOXYGEN
//! Implement operator<< on class name
/** The class named should define the method
    \c void \c show(std::ostream&).
 */
#define IMP_OUTPUT_OPERATOR(name)
#else
#define IMP_OUTPUT_OPERATOR(name)                                       \
  inline std::ostream &operator<<(std::ostream &out, const name &i)     \
  {                                                                     \
    i.show(out);                                                 \
    return out;                                                  \
  }
#endif



/** \name Swap helpers

    Use the swap_with member function to swap two objects. The two
    objects mustbe of the same type (Name) and define
    the method \c swap_with(). The number suffix is the number of template
    arguments, all of which must be of class type.
    @{
*/
#ifdef IMP_DOXYGEN
#define IMP_SWAP(Name)
#define IMP_SWAP_1(Name)
#define IMP_SWAP_2(Name)
#define IMP_SWAP_3(Name)
#define IMP_SWAP_4(Name)
#else
#define IMP_SWAP(Name)                                  \
  inline void swap(Name &a, Name &b) {a.swap_with(b);}

#define IMP_SWAP_1(Name)                                        \
  template <class A>                                            \
  inline void swap(Name<A> &a, Name<A> &b) {a.swap_with(b);}


#define IMP_SWAP_2(Name)                          \
  template <class A, class B>                     \
  inline void swap(Name<A,B> &a, Name<A,B> &b) {  \
    a.swap_with(b);                               \
  }

#define IMP_SWAP_3(Name)                                \
  template <class A, class B, class C>                  \
  inline void swap(Name<A,B,C> &a, Name<A,B,C> &b) {    \
    a.swap_with(b);                                     \
  }
#endif
/** @} */


//! swap two member variables assuming the other object is called o
/** Swap the member \c var_name of the two objects (this and o).
 */
#define IMP_SWAP_MEMBER(var_name) \
  std::swap(var_name, o.var_name);



//! use a copy_from method to create a copy constructor and operator=
/** This macro is there to aid with classes which require a custom
    copy constructor. It simply forwards \c operator= and the copy
    constructor to a method \c copy_from() which should do the copying.

    You should think very hard before implementing a class which
    requires a custom copy custructor as it is easy to get wrong
    and you can easily wrap most resources with RIIA objects
    (\external{en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization,
    wikipedia entry}).

 */
#define IMP_COPY_CONSTRUCTOR(TC) TC(const TC &o){copy_from(o);}  \
  TC& operator=(const TC &o) {copy_from(o); return *this;}



#ifdef IMP_DOXYGEN
//! Ref counted objects should have private destructors
/** This macro defines a private destructor and adds the appropriate
    friend methods so that the class can be used with ref counting.
    By defining a private destructor, you make it so that the object
    cannot be declared on the stack and so must be ref counted.

    \see IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR
    \see IMP::RefCounted
 */
#define IMP_REF_COUNTED_DESTRUCTOR(Name)
/** Like IMP_REF_COUNTED_DESTRUCTOR, but the destructor is only
    declared, not defined.
*/
#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)
#else

#ifdef _MSC_VER
// VC doesn't understand friends properly
#define IMP_REF_COUNTED_DESTRUCTOR(Name)                        \
public:                                                         \
 virtual ~Name(){}

#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)             \
public:                                                         \
 virtual ~Name()

#else

#if defined(SWIG) || defined(IMP_SWIG_WRAPPER)
// SWIG doesn't do friends right either, but we don't care as much
#define IMP_REF_COUNTED_DESTRUCTOR(Name)                     \
public:                                                      \
 virtual ~Name(){}
#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)     \
  public:                                               \
 virtual ~Name()


#else
/* The destructor is unprotected for SWIG since if it is protected
    SWIG does not wrap the python proxy distruction and so does not
    dereference the ref counted pointer. Swig also gets confused
    on template friends.
 */
#define IMP_REF_COUNTED_DESTRUCTOR(Name)                           \
  protected:                                                            \
  template <class T> friend void IMP::internal::unref(T*);              \
  virtual ~Name(){}

#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)                     \
  protected:                                                            \
  template <class T> friend void IMP::internal::unref(T*);              \
  virtual ~Name()

#endif // SWIG
#endif // _MSC_VER
#endif // doxygen


/** \name Macros to aid with implementing decorators

    These macros are here to aid in implementation of IMP::Decorator
    objects.  The first two declare/define the expected methods. The
    remainder help implement basic functions.

    @{
 */

//! Define the basic things needed by a Decorator.
/** The macro defines the following methods
    - a default constructor Decorator::Decorator()

    It also declares:
    - IMP::Decorator::show()
    - IMP::Decorator::decorate_particle()
    - IMP::Decorator::Decorator()

    Finally, it expects methods corresponding to
    - IMP::Decorator::particle_is_instance()
    - IMP::Decorator::setup_particle()

    You also implement static methods \c get_x_key() to return each of the
    keys used. These static methods, which must be defined in the \c .cpp
    file should declare the key itself as a \c static member variable to
    avoid initializing the key if the decorator is not used.

    See \ref decorators "the decorators page" for a more detailed description
    of decorators.

    \param[in] Name is the name of the decorator, such as XYZR
    \param[in] Parent The class name for the parent of this class,
    typically Decorator

    \see IMP_DECORATOR_TRAITS()
 */
#define IMP_DECORATOR(Name, Parent)                                     \
public:                                                                 \
/* Should be private but SWIG accesses it through the comparison
    macros*/                                                            \
IMP_NO_DOXYGEN(typedef Name This);                                      \
/** \brief Create null decorator. Such a decorator is like a NULL
    pointer in C++. */                                                  \
 Name(): Parent(){}                                                     \
/** \brief Create a decorator wrapping a particle which already has
    had setup_particle() called on it. */                               \
explicit Name(::IMP::Particle *p): Parent(p) {                          \
  IMP_INTERNAL_CHECK(particle_is_instance(p),                           \
              "Particle missing required attributes for decorator "     \
              << #Name << *p << std::endl);                             \
 }                                                                      \
static Name decorate_particle(::IMP::Particle *p) {                     \
   IMP_CHECK_OBJECT(p);                                                 \
   if (!particle_is_instance(p)) {                                      \
     return Name();                                                     \
   }                                                                    \
   return Name(p);                                                      \
 }                                                                      \
IMP_SHOWABLE


//! Define the basic things needed by a Decorator which has a traits object.
/** This macro is the same as IMP_DECORATOR() except that an extra object
    of type TraitsType is passed after the particle to
    - IMP::Decorator::particle_is_instance()
    - IMP::Decorator::setup_particle()
    - IMP::Decorator::decorate_particle()

    As in the IMP::core::XYZR or IMP::core::Hierarchy,
    this object can be used to parameterize the Decorator. The traits
    object is stored in the decorator and made accessible through
    the get_traits() method.
 */
#define IMP_DECORATOR_TRAITS(Name, Parent, TraitsType, traits_name,     \
default_traits)                                                         \
  private:                                                              \
  TraitsType traits_name##_;                                            \
public:                                                                 \
IMP_NO_DOXYGEN(typedef Name This;)                                      \
 /** \brief Create null decorator. Such a decorator is like a NULL
     pointer in C++. */                                                 \
Name(): Parent(){}                                                      \
/** \brief Create a decorator wrapping a particle which already has
    had setup_particle() called on it with the passed traits. */        \
Name(::IMP::Particle *p, const TraitsType &tr=default_traits): Parent(p), \
                                                   traits_name##_(tr) { \
  IMP_INTERNAL_CHECK(particle_is_instance(p, tr),                       \
             "Particle missing required attributes for decorator "      \
             << #Name << *p << std::endl);                              \
}                                                                       \
static Name decorate_particle(::IMP::Particle *p,                       \
                              const TraitsType &tr=default_traits) {    \
  if (!particle_is_instance(p, tr)) return Name();                      \
  else return Name(p, tr);                                              \
}                                                                       \
IMP_SHOWABLE;                                                           \
/** Get the traits object */                                            \
const TraitsType &get_##traits_name() const {                           \
  return traits_name##_;                                                \
}                                                                       \


//! Perform actions dependent on whether a particle has an attribute.
/** A common pattern is to check if a particle has a particular attribute,
    do one thing if it does and another if it does not. This macro implements
   that pattern. It requires that the method get_particle() return the
   particle being used.

   \param[in] AttributeKey The key for the attribute
   \param[in] Type The type for the attribute ("Int", "Float", "String")
   \param[in] has_action The action to take if the Particle has the attribute.
                         The attribute value is stored in the variable VALUE.
   \param[in] not_has_action The action to take if the Particle does not have
                             the attribute.
   \see IMP_DECORATOR_GET()
   \see IMP_DECORATOR_GET_SET()

 */
#define IMP_DECORATOR_GET(AttributeKey, Type, has_action, not_has_action) \
  if (get_particle()->has_attribute(AttributeKey)) {                    \
    Type VALUE =  get_particle()->get_value(AttributeKey);              \
    has_action;                                                         \
  } else {                                                              \
    not_has_action;                                                     \
  }


//! Set an attribute, creating it if it does not already exist.
/** Another common pattern is to have an assumed value if the attribute
    is not there. Then, you sometimes need to set the value whether it
    is there or not.
   \see IMP_DECORATOR_GET()
   \see IMP_DECORATOR_GET_SET()
 */
#define IMP_DECORATOR_SET(AttributeKey, value)          \
  if (get_particle()->has_attribute(AttributeKey)) {    \
    get_particle()->set_value(AttributeKey, value)  ;   \
  } else {                                              \
    get_particle()->add_attribute(AttributeKey, value); \
  }

//! define methods for getting and setting a particular simple field
/**
   This macros defines methods to get an set a particular attribute.

   \param[in] name The lower case name of the attribute
   \param[in] AttributeKey The AttributeKey object controlling
                           the attribute.
   \param[in] Type The type of the attribute (upper case).
   \param[in] ReturnType The type to return from the get.
   \see IMP_DECORATOR_GET()
   \see IMP_DECORATOR_SET()
*/
#define IMP_DECORATOR_GET_SET(name, AttributeKey, Type, ReturnType)     \
  ReturnType get_##name() const {                                       \
    return static_cast<ReturnType>(get_particle()->get_value(AttributeKey)); \
  }                                                                     \
  void set_##name(ReturnType t) {                                       \
    get_particle()->set_value(AttributeKey, t);                         \
  }

//! Define methods for getting and setting an optional simple field.
/**
   See IMP_DECORATOR_GET_SET(). The difference is that here you can provide
   a default value to use if the decorator does not have the attribute.

   \param[in] name The lower case name of the attribute
   \param[in] AttributeKey The expression to get the required attribute key.
   \param[in] Type The type of the attribute (upper case).
   \param[in] ReturnType The type to return from the get.
   \param[in] default_value The value returned if the attribute is missing.
 */
#define IMP_DECORATOR_GET_SET_OPT(name, AttributeKey, Type,             \
                                  ReturnType, default_value)            \
  ReturnType get_##name() const {                                       \
    IMP_DECORATOR_GET(AttributeKey, Type,                               \
                      return static_cast<ReturnType>(VALUE),            \
                      return default_value);                            \
  }                                                                     \
  void set_##name(ReturnType t) {                                       \
    IMP_DECORATOR_SET(AttributeKey, t);                                 \
  }


//! Define a set of attributes which form an array
/**
   The macro defines a set of functions for using the array:
   - get_name(unsigned int)

   - get_number_of_name()

   - add_name(ExternalType)

   - add_name_at(ExternalType, unsigned int)

   - remove_name(unsigned int)

   in addition it defines the private methods
   - add_required_attributes_for_name(Particle *)

   \param[in] protection Whether it should be public, protected or private
   \param[in] Class The name of the wrapping class
   \param[in] Name The capitalized name of the prefix to use
   \param[in] name the name prefix to use, see the above method names
   \param[in] plural the plural form of the name
   \param[in] traits the traits object to use to manipulate things. This should
   inherit from or implement the interface of internal::ArrayOnAttributesHelper
   \param[in] ExternalType The name of the type to wrap the return type with.
   \param[in] ExternalTypes A vector of the return type.
 */
#define IMP_DECORATOR_ARRAY_DECL(protection, Class,                     \
                                 Name, name, plural,                    \
                                 traits, ExternalType, ExternalTypes)   \
private:                                                                \
 template <class T>                                                     \
 static bool has_required_attributes_for_##name(Particle *p,            \
                                                const T &traits) {      \
  return traits.has_required_attributes(p);                             \
 }                                                                      \
 template <class T>                                                     \
 static void add_required_attributes_for_##name(Particle *p,            \
                                                const T &traits) {      \
   return traits.add_required_attributes(p);                            \
 }                                                                      \
 struct Name##AttrArrayAccessor {                                       \
   const Class &d_;                                                     \
   Name##AttrArrayAccessor(const Class &d): d_(d){}                     \
   typedef ExternalType result_type;                                    \
   result_type operator()(unsigned int i) const {                       \
     return d_.get_##name(i);                                           \
   }                                                                    \
   bool operator==(const Name##AttrArrayAccessor &o) const {            \
     return d_== o.d_;                                                  \
   }                                                                    \
 };                                                                     \
protection:                                                             \
 IMP_NO_SWIG(typedef IMP::internal                                      \
             ::IndexingIterator<Name##AttrArrayAccessor>                \
             Name##Iterator;)                                           \
 IMP_NO_SWIG(Name##Iterator plural##_begin() const {                    \
     return Name##Iterator(Name##AttrArrayAccessor(*this));             \
 }                                                                      \
 Name##Iterator plural##_end() const {                                  \
   return Name##Iterator(Name##AttrArrayAccessor(*this),                \
                         get_number_of_##plural());                     \
 })                                                                     \
 ExternalType get_##name(unsigned int i) const {                        \
   return traits.wrap(traits.get_value(get_particle(), i));             \
 }                                                                      \
 unsigned int get_number_of_##plural() const {                          \
   return traits.get_size(get_particle());                              \
 }                                                                      \
 unsigned int add_##name(ExternalType t) {                              \
   traits.audit_value(t);                                               \
   unsigned int i= traits.push_back(get_particle(),                     \
                                    traits.get_value(t));               \
   traits.on_add(get_particle(), t, i);                                 \
   return i;                                                            \
 }                                                                      \
 void add_##name##_at(ExternalType t, unsigned int idx) {               \
   traits.audit_value(t);                                               \
   traits.insert(get_particle(),                                        \
                 idx,                                                   \
                 traits.get_value(t));                                  \
   traits.on_add(get_particle(), t, idx);                               \
   for (unsigned int i= idx+1; i < get_number_of_##plural(); ++i) {     \
     traits.on_change(get_particle(),                                   \
                      traits.get_value( get_particle(), i),             \
                      i-1, i);                                          \
   }                                                                    \
 }                                                                      \
 ExternalTypes get_##plural() const {                                   \
   ExternalTypes ret;                                                   \
   for (unsigned int i=0; i< get_number_of_##plural(); ++i) {           \
     ret.push_back(get_##name(i));                                      \
   }                                                                    \
   return ret;                                                          \
 }                                                                      \
 void add_##plural(const ExternalTypes &et) {                           \
   for (unsigned int i=0; i< et.size(); ++i) {                          \
     add_##name(et[i]);                                                 \
   }                                                                    \
 }                                                                      \
 void remove_##name(ExternalType t) {                                   \
   traits.audit_value(t);                                               \
   unsigned int idx= traits.get_index(get_particle(), t);               \
   traits.on_remove(get_particle(), t);                                 \
   traits.erase(get_particle(),                                         \
                      idx);                                             \
   for (unsigned int i= idx; i < get_number_of_##plural(); ++i) {       \
     traits.on_change(get_particle(),                                   \
                       traits.get_value(get_particle(), i),             \
                       i+1, i);                                         \
   }                                                                    \
 }                                                                      \
 void clear_##plural() {                                                \
   traits.clear(get_particle());                                        \
 }

//! @}



/** \defgroup object_helpers Macros to aid with implementation classes

    These macros are here to aid with implementing classes that
    inherit from the various abstract base classes in the kernel. Each
    macro, which should be used in the body of the class,
    declares/defines the set of needed functions. The declared
    functions should be defined in the associated \c .cpp file. By
    using the macros, you ensure that your class gets the names of the
    functions correct and it makes it easier to update your class if
    the functions should change.

    All of the macros define the following methods:
    - IMP::Object::get_version_info()
    - an empty virtual destructor

    In addition, they all declare:
    - IMP::Object::show()

    For all macros, the Name paramete is the name of the class being
    implemented and the version_info parameter is the IMP::VersionInfo
    to use (probably get_version_info()).

    @{
 */


//! Define the basic things needed by any Object
/**
   This defines
   - IMP::Object::get_version_info()
   - a private destructor
   and declares
   - IMP::Object::show()
 */
#define IMP_OBJECT(Name, version_info)                                  \
  public:                                                               \
  virtual void show(std::ostream &out=std::cout) const;                 \
  virtual ::IMP::VersionInfo get_version_info() const { return version_info; } \
  IMP_REF_COUNTED_DESTRUCTOR(Name)                                      \
  public:

//! Define the basic things needed by any internal Object
/** \see IMP_OBJECT
    This version also defines IMP::Object::show()
 */
#define IMP_INTERNAL_OBJECT(Name, version_info)                         \
  public:                                                               \
  virtual void show(std::ostream &out=std::cout) const {\
    out << #Name << std::endl;}                                         \
  virtual ::IMP::VersionInfo get_version_info() const { return version_info; } \
  IMP_REF_COUNTED_DESTRUCTOR(Name);                                     \
  public:


//! Define the basic things needed for an Interaction object
/** In addition to the types needed by IMP::Object (see IMP_OBJECT),
    it declares:
    - IMP::Interaction::get_interacting_particles()
    - IMP::Interaction::get_read_particles()
    - IMP::Interaction::get_write_particles()
*/
#define IMP_INTERACTION(Name, version_info)     \
  ParticlesList get_interacting_particles() const;                      \
  ParticlesTemp get_read_particles() const;                             \
  ParticlesTemp get_write_particles() const;                            \
  IMP_OBJECT(Name, version_info)

//! Define the basic things you need for a Restraint.
/** In addition to the methods defined by IMP::Interaction
    (see IMP_INTERACTION), it declares
    - IMP::Restraint::unprotected_evaluate()

    It also defines
    - IMP::Restraint::get_is_incremental() to return 0
    - IMP::Restraint::incremental_evaluate() to throw an exception
*/
#define IMP_RESTRAINT(Name, version_info)                               \
  virtual double unprotected_evaluate(DerivativeAccumulator *accum) const;   \
  IMP_INTERACTION(Name, version_info);

//! Define the basic things you need for a Restraint.
/** In addition to the methods done by IMP_INTERACTION, it declares
    - IMP::Restraint::unprotected_evaluate()
    - IMP::Restraint::unprotected_incremental_evaluate()

    and it defines
    - IMP::Restraint::get_is_incremental() to return true
*/
#define IMP_INCREMENTAL_RESTRAINT(Name, version_info)                   \
  virtual double unprotected_evaluate(DerivativeAccumulator *accum) const; \
  virtual bool get_is_incremental() const {return true;}                \
  virtual double                                                        \
  unprotected_incremental_evaluate(DerivativeAccumulator *accum) const; \
  IMP_INTERACTION(Name, version_info);

//! Define the basic things you need for an optimizer.
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::Optimizer::optimize()

    \relatesalso IMP::Optimizer
*/
#define IMP_OPTIMIZER(Name, version_info)                               \
  virtual Float optimize(unsigned int max_steps);                       \
  IMP_OBJECT(Name, version_info);


//! Define the basics needed for an OptimizerState
/** In addition to the methods done IMP_OBJECT, it declares
    - IMP::OptimizerState::update()
*/
#define IMP_OPTIMIZER_STATE(Name, version_info)                         \
  virtual void update();                                                \
  IMP_OBJECT(Name, version_info);


//! Define the basics needed for an OptimizerState which acts every n steps
/** In addition to the methods done by all the macros, it declares
    - do_update(unsigned int call_number) where step number
    is the number of the optimization step, and call_number is the number
    of the call to do_update.
    It also defines
    - void set_skip_steps(unsigned int)

    If you use this macro, you should also include IMP/internal/utility.h.
*/
#define IMP_PERIODIC_OPTIMIZER_STATE(Name, version_info)                \
  virtual void update() {                                               \
    if (call_number_%(skip_steps_+1) ==0) {                             \
      do_update(update_number_);                                        \
      ++update_number_;                                                 \
    }                                                                   \
    ++call_number_;                                                     \
  }                                                                     \
  void do_update(unsigned int call_number);                             \
  void set_skip_steps(unsigned int k) {                                 \
    skip_steps_=k;                                                      \
    call_number_=0;                                                     \
  }                                                                     \
  IMP_OBJECT(Name, version_info)                                        \
  private:                                                              \
  ::IMP::internal::Counter skip_steps_, call_number_, update_number_;   \


//! Define the basics needed for a ScoreState
/** In addition to the methods done by IMP_INTERACTION, it declares
    - IMP::ScoreState::do_before_evaluate()
    - IMP::ScoreState::do_after_evaluate()
*/
#define IMP_SCORE_STATE(Name, version_info)                             \
protected:                                                              \
 virtual void do_before_evaluate();                                     \
 virtual void do_after_evaluate(DerivativeAccumulator *da);             \
 IMP_INTERACTION(Name, version_info)


//! Define the basics needed for a particle refiner
/** In addition to the methods done by all the macros, it declares
    - IMP::Refiner::get_can_refine()
    - IMP::Refiner::get_number_of_refined()
    - IMP::Refiner::get_refined();

    \see IMP_SIMPLE_REFINER
 */
#define IMP_REFINER(Name, version_info)                                 \
  virtual bool get_can_refine(Particle*) const;                         \
  virtual Particle* get_refined(Particle *, unsigned int) const;        \
  virtual const ParticlesTemp get_refined(Particle *) const;            \
  virtual unsigned int get_number_of_refined(Particle *) const;         \
  IMP_OBJECT(Name, version_info);


//! Define the basics needed for a particle refiner
/** In contrast to IMP_REFINER, if this macro is used, the
    Refiner::get_refined(Particle*) method is implemented using the
    other Refiner::get_refined() method and so does not have to be
    provided.

    \see IMP_REFINER
 */
#define IMP_SIMPLE_REFINER(Name, version_info)                          \
  virtual bool get_can_refine(Particle*) const;                         \
  virtual Particle* get_refined(Particle *, unsigned int) const;        \
  virtual const ParticlesTemp get_refined(Particle *a) const {           \
    ParticlesTemp ret(get_number_of_refined(a));                        \
    for (unsigned int i=0; i< ret.size(); ++i) {                        \
      ret[i]= get_refined(a,i);                                         \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  virtual unsigned int get_number_of_refined(Particle *) const;         \
  IMP_OBJECT(Name, version_info);



//! Declare the functions needed for a SingletonScore
/** In addition to the methods done by IMP_INTERACTON, it declares
    - IMP::SingletonScore::evaluate(IMP::Particle*,
    IMP::DerivativeAccumulator*)
    - IMP::SingletonScore::get_interacting_particles()
    IMP::DerivativeAccumulator*)
    - IMP::SingletonScore::get_read_particles()
    - IMP::SingletonScore::get_write_particles()
 */
#define IMP_SINGLETON_SCORE(Name, version_info)                  \
  double evaluate(Particle *a, DerivativeAccumulator *da) const; \
  double evaluate(const ParticlesTemp &ps,                       \
                  DerivativeAccumulator *da) const {             \
    double ret=0;                                                \
    for (unsigned int i=0; i< ps.size(); ++i) {                  \
      ret+=Name::evaluate(ps[i], da);                            \
    }                                                            \
    return ret;                                                  \
  }                                                              \
  ParticlesList get_interacting_particles(Particle*) const;      \
  ParticlesTemp get_read_particles(Particle*) const;             \
  ParticlesTemp get_write_particles(Particle*) const;            \
  IMP_OBJECT(Name, version_info);


//! Declare the functions needed for a PairScore
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::PairScore::evaluate(IMP::Particle*,IMP::Particle*,
es
    - IMP::PairScore::get_interacting_particles()
    IMP::DerivativeAccumulator*)
    - IMP::PairScore::get_read_particles()
    - IMP::PairScore::get_write_particles()
*/
#define IMP_PAIR_SCORE(Name, version_info)                              \
  double evaluate(Particle *a, Particle *b,                             \
                  DerivativeAccumulator *da) const;                     \
  double evaluate(const ParticlePairsTemp &ps,                          \
                  DerivativeAccumulator *da) const {                    \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate(ps[i][0], ps[i][1], da);                      \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  ParticlesList get_interacting_particles(Particle*, Particle*) const;  \
  ParticlesTemp get_read_particles(Particle*, Particle*) const;         \
  ParticlesTemp get_write_particles(Particle*, Particle*) const;        \
  IMP_OBJECT(Name, version_info);


//! Declare the functions needed for a SingletonModifier
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::SingletonModifier::apply(IMP::Particle*)
    - IMP::SingletonModifier::get_interacting_particles()
    - IMP::SingletonModifier::get_read_particles()
    - IMP::SingletonModifier::get_write_particles()
    \see IMP_SINGLETON_MODIFIER_DA
 */
#define IMP_SINGLETON_MODIFIER(Name, version_info)                      \
  void apply(Particle *a) const;                                        \
  void apply(Particle *a, DerivativeAccumulator&) const{                \
    apply(a);                                                           \
  }                                                                     \
  void apply(const ParticlesTemp &ps) const {                           \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i]);                                               \
    }                                                                   \
  }                                                                     \
  void apply(const ParticlesTemp &ps, DerivativeAccumulator &) const {  \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i]);                                               \
    }                                                                   \
  }                                                                     \
  ParticlesList get_interacting_particles(Particle*) const;             \
  ParticlesTemp get_read_particles(Particle*) const;                    \
  ParticlesTemp get_write_particles(Particle*) const;                   \
  IMP_OBJECT(Name, version_info);



//! Declare the functions needed for a PairModifier
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::PairModifier::apply(IMP::Particle*,IMP::Particle*)
    - IMP::PairModifier::get_interacting_particles()
    - IMP::PairModifier::get_read_particles()
    - IMP::PairModifier::get_write_particles()
    \see IMP_PAIR_MODIFIER_DA
*/
#define IMP_PAIR_MODIFIER(Name,version_info)                            \
  void apply(Particle *a, Particle *b) const;                           \
  void apply(Particle *a, Particle *b, DerivativeAccumulator&) const{   \
    apply(a,b);                                                         \
  }                                                                     \
  void apply(const ParticlePairsTemp &ps) const {                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i][0], ps[i][1]);                                  \
    }                                                                   \
  }                                                                     \
  void apply(const ParticlePairsTemp &ps,                               \
             DerivativeAccumulator &) const {                           \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i][0], ps[i][1]);                                  \
    }                                                                   \
  }                                                                     \
  ParticlesList get_interacting_particles(Particle*, Particle*) const;  \
  ParticlesTemp get_read_particles(Particle*, Particle*) const;         \
  ParticlesTemp get_write_particles(Particle*, Particle*) const;        \
  IMP_OBJECT(Name, version_info);


//! Declare the functions needed for a SingletonModifier
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::SingletonModifier::apply(IMP::Particle*,
    IMP::DerivativeAccumulator&)
    - IMP::SingletonModifier::get_interacting_particles()
    - IMP::SingletonModifier::get_read_particles()
    - IMP::SingletonModifier::get_write_particles()

   \see IMP_SINGLETON_MODIFIER
 */
#define IMP_SINGLETON_MODIFIER_DA(Name, version_info)                   \
  void apply(Particle *a, DerivativeAccumulator &da) const;             \
  void apply(Particle *) const{                                         \
    IMP_LOG(VERBOSE, "This modifier requires a derivative accumulator " \
            << *this << std::endl);                                     \
  }                                                                     \
  void apply(const ParticlesTemp &ps) const {                           \
    IMP_LOG(VERBOSE, "This modifier requires a derivative accumulator " \
            << *this << std::endl);                                     \
  }                                                                     \
  void apply(const ParticlesTemp &ps,                                   \
             DerivativeAccumulator &da) const {                         \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i], da);                                           \
    }                                                                   \
  }                                                                     \
  ParticlesList get_interacting_particles(Particle*) const;             \
  ParticlesTemp get_read_particles(Particle*) const;                    \
  ParticlesTemp get_write_particles(Particle*) const;                   \
  IMP_OBJECT(Name, version_info);



//! Declare the functions needed for a PairModifier
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::PairModifier::apply(Particle*,Particle*,DerivativeAccumulator&)
    - IMP::PairModifier::get_interacting_particles()
    - IMP::PairModifier::get_read_particles()
    - IMP::PairModifier::get_write_particles()
    \see IMP_PAIR_MODIFIER
 */
#define IMP_PAIR_MODIFIER_DA(Name, version_info)                        \
 void apply(Particle *a, Particle *b, DerivativeAccumulator *da) const; \
 void apply(Particle *, Particle *) const{                              \
   IMP_LOG(VERBOSE, "This modifier requires a derivative accumulator "  \
           << *this << std::endl);                                      \
 }                                                                      \
 void apply(const ParticlePairsTemp &ps) const {                        \
   IMP_LOG(VERBOSE, "This modifier requires a derivative accumulator "  \
           << *this << std::endl);                                      \
 }                                                                      \
 void apply(const ParticlePairsTemp &ps,                                \
            DerivativeAccumulator &da) const {                          \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i][0], ps[i][1], da);                              \
    }                                                                   \
 }                                                                      \
 ParticlesList get_interacting_particles(Particle*, Particle*) const;   \
 ParticlesTemp get_read_particles(Particle*, Particle*) const;          \
 ParticlesTemp get_write_particles(Particle*, Particle*) const;         \
 IMP_OBJECT(Name, version_info)



//! Define a simple SingletonModifier
/** In addition to the methods done by IMP_OBJECT, it defines
    - IMP::SingletonModifier::apply(IMP::Particle*) to the provided value
    - IMP::SingletonModifier::get_interacting_particles() to return None
    - IMP::SingletonModifier::get_read_particles() to return the particle
    - IMP::SingletonModifier::get_write_particles() to return the particle
    - IMP::Object::show()
    This macro should only be used to define types which are used
    internally in algorithms and data structures.
    \see IMP_SINGLETON_MODIFIER_DA
    \see IMP_SINGLETON_MODIFIER
 */
#define IMP_INTERNAL_SINGLETON_MODIFIER(Name, version_info,             \
                                        apply_expr)                     \
  void apply(Particle *p) const {                                       \
    apply_expr;                                                         \
  }                                                                     \
  void apply(Particle *a, DerivativeAccumulator&) const{                \
    apply(a);                                                           \
  }                                                                     \
  void apply(const ParticlesTemp &ps) const {                           \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i]);                                               \
    }                                                                   \
  }                                                                     \
  void apply(const ParticlesTemp &ps, DerivativeAccumulator &) const {  \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i]);                                               \
    }                                                                   \
  }                                                                     \
  ParticlesList get_interacting_particles(Particle*) const {            \
    return ParticlesList();                                             \
  }                                                                     \
  ParticlesTemp get_read_particles(Particle*p) const {                  \
    return ParticlesTemp(1,p);                                          \
  }                                                                     \
  ParticlesTemp get_write_particles(Particle*p) const {                 \
    return ParticlesTemp(1,p);                                          \
  }                                                                     \
  IMP_INTERNAL_OBJECT(Name, version_info);



//! Declare the needed functions for a SingletonContainer
/** In addition to the methods done by all the macros, it declares
    - IMP::SingletonContainer::get_contains_particle()
    - IMP::SingletonContainer::get_number_of_particles()
    - IMP::SingletonContainer::get_particle()
    - IMP::SingletonContainer::apply()
    - IMP::SingletonContainer::evaluate()
    - IMP::SingletonContainer::get_particles()
    - IMP::SingletonContainer::get_revision()
*/
#define IMP_SINGLETON_CONTAINER(Name, version_info)                 \
  bool get_contains_particle(Particle* p) const;                    \
  unsigned int get_number_of_particles() const;                     \
  Particle* get_particle(unsigned int i) const;                     \
  void apply(const SingletonModifier *sm);                          \
  void apply(const SingletonModifier *sm,                           \
             DerivativeAccumulator &da);                            \
  double evaluate(const SingletonScore *s,                          \
                  DerivativeAccumulator *da) const;                 \
  ParticlesTemp get_particles() const;                              \
  unsigned int get_revision() const;                                \
  IMP_OBJECT(Name, version_info);


//! Declare the needed functions for a PairContainer
/** In addition to the methods done by all the macros, it declares
    - IMP::PairContainer::get_contains_particle_pair()
    - IMP::PairContainer::get_number_of_particle_pairs()
    - IMP::PairContainer::get_particle_pair()
    - IMP::PairContainer::apply()
    - IMP::PairContainer::evaluate()
    - IMP::PairContainer::get_particle_pairs()
    - IMP::PairContainer::get_revision()
 */
#define IMP_PAIR_CONTAINER(Name, version_info)                          \
  bool get_contains_particle_pair(ParticlePair p) const;                \
  unsigned int get_number_of_particle_pairs() const;                    \
  ParticlePair get_particle_pair(unsigned int i) const;                 \
  void apply(const PairModifier *sm);                                   \
  void apply(const PairModifier *sm,                                    \
             DerivativeAccumulator &da);                                \
  double evaluate(const PairScore *s,                                   \
                  DerivativeAccumulator *da) const;                     \
  unsigned int get_revision() const;                                    \
  ParticlePairsTemp get_particle_pairs() const;                         \
  IMP_OBJECT(Name, version_info)



//! Declare the needed functions for a SingletonFilter
/** In addition to the methods done by all the macros, it declares
    - IMP::SingletonFilter::get_contains_particle()
    - IMP::SingletonFilter::get_used_particles()
*/
#define IMP_SINGLETON_FILTER(Name, version_info)                    \
  bool get_contains_particle(Particle* p) const;                    \
  ParticlesTemp get_used_particles(const ParticlesTemp &t) const;   \
  IMP_OBJECT(Name, version_info)


//! Declare the needed functions for a PairFilter
/** In addition to the methods done by all the macros, it declares
    - IMP::PairFilter::get_contains_particle_pair()
    - IMP::PairFilter::get_used_particles(const ParticlesTemp &t)
 */
#define IMP_PAIR_FILTER(Name, version_info)                             \
  bool get_contains_particle_pair(ParticlePair p) const;                \
  ParticlesTemp get_used_particles(const ParticlePairsTemp &t) const;   \
  IMP_OBJECT(Name, version_info);



//! Declare the needed functions for a UnaryFunction
/** In addition to the methods done by all the macros, it declares
    - IMP::UnaryFunction::evaluate()
    - IMP::UnaryFunction::evaluate_with_derivatives()

    \see IMP_UNARY_FUNCTION_INLINE
*/
#define IMP_UNARY_FUNCTION(Name, version_info)                          \
  virtual DerivativePair evaluate_with_derivative(double feature) const; \
  virtual double evaluate(double feature) const;                        \
  IMP_OBJECT(Name, version_info)



//! Declare the needed functions for a UnaryFunction which evaluates inline
/** This macro declares all the functions needed for an IMP::UnaryFunction
    inline in the class. There is no need for an associated \c .cpp file.

    The last three arguments are expressions that evaluate to the
    unary function value and derivative and are sent to the stream in the
    show function, respectively. The input to the function is called
    \c feature.

    \see IMP_UNARY_FUNCTION
*/
#define IMP_UNARY_FUNCTION_INLINE(Name, version_info, value_expression, \
                           derivative_expression, show_expression)      \
  virtual DerivativePair evaluate_with_derivative(double feature) const { \
    return DerivativePair((value_expression), (derivative_expression)); \
  }                                                                     \
  virtual double evaluate(double feature) const {                       \
    return (value_expression);                                          \
  }                                                                     \
  virtual void show(std::ostream &out=std::cout) const {                \
    out << show_expression;                                             \
  }                                                                     \
  ::IMP::VersionInfo get_version_info() const { return version_info; }  \
  IMP_REF_COUNTED_DESTRUCTOR(Name);                                     \
  public:


//! Declare a IMP::FailureHandler
/** In addition to the standard methods it declares:
    - IMP::FailureHandler::handle_failure()
*/
#define IMP_FAILURE_HANDLER(Name, version_info)                 \
  void handle_failure();                                        \
  IMP_OBJECT(Name, version_info)



//! Declare an IMP RAII-style function
/** Since such classes are typically quite small and simple, all
    the implementation is inline. The macro declares
    - constructor
    - RAII::set()
    - RAII::reset()
    - destructor
    The Set and Reset arguments are the code to run in the respective
    functions. "args" are the argument string (in parens) for the
    constructor and set.
 */
#define IMP_RAII(Name, args, Initialize, Set, Reset)   \
  Name() {Initialize;}                                 \
  explicit Name args {Initialize; Set}                 \
  void set args {reset();                              \
    Set;}                                              \
  void reset() {Reset;}                                \
  ~Name () {reset();}


//! Declare the methods needed by an object than can be printed
/** This macro declares the method
    - void show(std::ostream &out) const
    It also makes it so that the object can be printed
    in python.

    The \c ostream and \c sstream headers must be included.

    See also IMP_SHOWABLE_INLINE().

    Do not use with IMP::Object objects as they have their
    own show mechanism.
*/
#ifndef IMP_DOXYGEN
#define IMP_SHOWABLE                            \
  void show(std::ostream &out=std::cout) const; \
  std::string __str__() const {                 \
    std::ostringstream out;                     \
    show(out);                                  \
    return out.str();                           \
  }
#else
#define IMP_SHOWABLE                            \
  void show(std::ostream &out=std::cout) const;
#endif

//! Declare the methods needed by an object than can be printed
/** This macro declares the method
    - \c void \c show(std::ostream &out) const
    It also makes it so that the object can be printed
    in python.

    The \c ostream and \c sstream headers must be included.

    See also IMP_SHOWABLE_INLINE()
*/
#ifndef IMP_DOXYGEN
#define IMP_SHOWABLE_INLINE(how_to_show)        \
  void show(std::ostream &out=std::cout) const{ \
    how_to_show;                                \
  }                                             \
  std::string __str__() const {                 \
    std::ostringstream out;                     \
    show(out);                                  \
    return out.str();                           \
  }
#else
#define IMP_SHOWABLE_INLINE(how_to_show)        \
  void show(std::ostream &out=std::cout) const;
#endif



//! @}

#ifndef IMP_DOXYGEN

//! Declare a ref counted pointer to a new object
/** \param[in] Typename The namespace qualified type being declared
    \param[in] varname The name for the ref counted pointer
    \param[in] args The paretesized arguments to the constructor, or ()
    if there are none.
    \see IMP::Pointer
    \code
    IMP_NEW(m, IMP::Model, ());
    \endcode
 */
#define IMP_NEW(Typename, varname, args)        \
  Pointer<Typename> varname(new Typename args)



/**
   Define a new key type.

   It defines two public types Name, which is an instantiation of KeyBase and
   Names which is a vector of Name.

   \param[in] Name The name for the new type.
   \param[in] Tag A (hopefully) unique integer to define this key type.

   \note We define a new class rather than use a typedef since SWIG has a
   bug dealing with names that start with ::. A fix has been commited to SVN
   for swig.

   \note The name in the typedef would have to start with ::IMP so it
   could be used out of the IMP namespace.
 */
#ifndef IMP_DOXYGEN
#define IMP_DECLARE_KEY_TYPE(Name, Tag)                                 \
  struct Name: public ::IMP::KeyBase<Tag, true> {                       \
    typedef ::IMP::KeyBase<Tag, true> P;                                \
  typedef Name This;                                                    \
  Name(){};                                                             \
  Name(unsigned int i): P(i){}                                          \
  Name(std::string nm): P(nm){}                                         \
  static Name add_alias(Name nm, std::string new_name) {                \
    ::IMP::KeyBase<Tag, true>:: add_alias(nm, new_name);                \
    IMP_INTERNAL_CHECK(Name(new_name) == nm,                            \
                       "Keys don't match after alias.");                \
    return Name(new_name);                                              \
  }                                                                     \
};                                                                      \
typedef std::vector<Name> Name##s
#else
#define IMP_DECLARE_KEY_TYPE(Name, Tag)                                 \
  /** A string based identifier.*/                                      \
  struct Name: public ::IMP::KeyBase<ID, true> {                        \
    typedef ::IMP::KeyBase<ID, true> P;                                 \
  typedef Name This;                                                    \
  Name(){};                                                             \
  IMP_NO_DOXYGEN(Name(unsigned int i): P(i){})                          \
  Name(std::string nm): P(nm){}                                         \
  /** Define the string new_name to refer to the same key as nm. */     \
  static Name add_alias(Name nm, std::string new_name);                 \
};                                                                      \
typedef std::vector<Name> Name##s
#endif


/**
   Define a new key non lazy type where new types have to be created
   explicitly.

   \see IMP_DECLARE_KEY_TYPE
 */
#ifndef IMP_DOXYGEN
#define IMP_DECLARE_CONTROLLED_KEY_TYPE(Name, Tag)                      \
  struct Name: public ::IMP::KeyBase<Tag, false> {                      \
    typedef ::IMP::KeyBase<Tag, false> P;                               \
  typedef Name This;                                                    \
  Name(){};                                                             \
  Name(unsigned int i): P(i){}                                          \
  Name(std::string nm): P(nm){}                                         \
  static Name add_alias(Name nm, std::string new_name) {                \
    ::IMP::KeyBase<Tag, false>:: add_alias(nm, new_name);               \
    IMP_INTERNAL_CHECK(Name(new_name) == nm,                            \
                       "Keys don't match after alias.");                \
    return Name(nm.get_index());                                        \
  }                                                                     \
};                                                                      \
typedef std::vector<Name> Name##s
#else
#define IMP_DECLARE_CONTROLLED_KEY_TYPE(Name, Tag)                      \
  /** A string based identifier.*/                                      \
  struct Name: public ::IMP::KeyBase<ID, false> {                       \
    typedef ::IMP::KeyBase<ID, false> P;                                \
  typedef Name This;                                                    \
  Name(){};                                                             \
  Name(std::string nm): P(nm){}                                         \
  IMP_NO_DOXYGEN(Name(unsigned int i): P(i){})                          \
  /** Define the string new_name to refer to the same key as nm. */     \
  static Name add_alias(Name nm, std::string new_name);                 \
};                                                                      \
typedef std::vector<Name> Name##s
#endif



//! Use this to label a function with no side effects
#ifdef __GNU__
#define IMP_NO_SIDEEFFECTS __attribute__ ((pure))
#else
#define IMP_NO_SIDEEFFECTS
#endif


//! Use this to make the compiler (possibly) warn if the result is not used
#ifdef __GNU__
#define IMP_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
#else
#define IMP_WARN_UNUSED_RESULT
#endif

#endif

#endif  /* IMP_MACROS_H */
