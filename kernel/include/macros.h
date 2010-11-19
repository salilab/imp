/**
 *  \file IMP/macros.h    \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_MACROS_H
#define IMP_MACROS_H

#ifdef IMP_DOXYGEN
#define IMP_REQUIRE_SEMICOLON_CLASS(Name)
#define IMP_REQUIRE_SEMICOLON_NAMESPACE
#elif defined(SWIG)
#define IMP_REQUIRE_SEMICOLON_CLASS(Name)
#define IMP_REQUIRE_SEMICOLON_NAMESPACE
#else
#define IMP_REQUIRE_SEMICOLON_CLASS(Name)                       \
  IMP_NO_DOXYGEN(IMP_NO_SWIG(friend void dummy_f_##Name()))
#define IMP_REQUIRE_SEMICOLON_NAMESPACE void dummy_f()
#endif

#ifdef IMP_DOXYGEN
//! Hide something from doxygen
/** */
#define IMP_NO_DOXYGEN(x)
//! Only show something to doxygen
/** */
#define IMP_ONLY_DOXYGEN(x) x IMP_REQUIRE_SEMICOLON_CLASS(only_doxy)
#define IMP_SWITCH_DOXYGEN(x,y) x
#else
#define IMP_NO_DOXYGEN(x) x
#define IMP_ONLY_DOXYGEN(x) IMP_REQUIRE_SEMICOLON_CLASS(only_doxy)
#define IMP_SWITCH_DOXYGEN(x,y) y
#endif

#if defined(SWIG)
#define IMP_NO_SWIG(x)
#else
//! Hide the line when SWIG is compiled or parses it
/** */
#define IMP_NO_SWIG(x) x
#endif

#if defined(IMP_DOXYGEN)
/** \name Comparisons
    Helper macros for implementing comparisons in terms of
    either member variables or a member compare function.
    All of the <,>,== etc are implemented for both C++
    and Python.
    @{
*/

//! Implement comparison in a class using a compare function
/** The macro requires that This be defined as the type of the current class.
    The compare function should take a const This & and return -1, 0, 1 as
    appropriate.
*/
#define IMP_COMPARISONS

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_1(field)

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_2(f0, f1)

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_3(f0, f1, f2)
/** @} */
#elif defined(SWIG)
#define IMP_SWIG_COMPARISONS                                            \
  bool __eq__(const This &o) const;                                     \
  bool __ne__(const This &o) const;                                     \
  bool __lt__(const This &o) const;                                     \
  bool __gt__(const This &o) const;                                     \
  bool __ge__(const This &o) const;                                     \
  bool __le__(const This &o) const


#define IMP_COMPARISONS                         \
  IMP_SWIG_COMPARISONS

#define IMP_COMPARISONS_1(field)                \
  IMP_SWIG_COMPARISONS

#define IMP_COMPARISONS_2(f0, f1)               \
  IMP_SWIG_COMPARISONS

#define IMP_COMPARISONS_3(f0, f1, f2)           \
  IMP_SWIG_COMPARISONS

#else // not doxygen
#define IMP_SWIG_COMPARISONS                                            \
  bool __eq__(const This &o) const {                                    \
    return operator==(o);                                               \
  }                                                                     \
  bool __ne__(const This &o) const {                                    \
    return operator!=(o);                                               \
  }                                                                     \
  bool __lt__(const This &o) const {                                    \
    return operator<(o);                                                \
  }                                                                     \
  bool __gt__(const This &o) const {                                    \
    return operator>(o);                                                \
  }                                                                     \
  bool __ge__(const This &o) const {                                    \
    return operator>=(o);                                               \
  }                                                                     \
  bool __le__(const This &o) const {                                    \
    return operator<=(o);                                               \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_CLASS(comparisons)

#define IMP_COMPARISONS                                                 \
  IMP_SWIG_COMPARISONS;                                                 \
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
  template <class T> friend int compare(const T&a, const T&b)

#define IMP_COMPARISONS_1(field)                \
  bool operator==(const This &o) const {        \
    return (field== o.field);                   \
  }                                             \
  bool operator!=(const This &o) const {        \
    return (field!= o.field);                   \
  }                                             \
  bool operator<(const This &o) const {         \
    return (field< o.field);                    \
  }                                             \
  bool operator>(const This &o) const {         \
    return (field> o.field);                    \
  }                                             \
  bool operator>=(const This &o) const {        \
    return (field>= o.field);                   \
  }                                             \
  bool operator<=(const This &o) const {        \
    return (field<= o.field);                   \
  }                                             \
  int compare(const This &o) const {            \
    if (operator<(o)) return -1;                \
    else if (operator>(o)) return 1;            \
    else return 0;                              \
  }                                             \
  IMP_SWIG_COMPARISONS

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_2(f0, f1)               \
  bool operator==(const This &o) const {        \
    return (f0== o.f0 && f1==o.f1);             \
  }                                             \
  bool operator!=(const This &o) const {        \
    return (f0!= o.f0 || f1 != o.f1);           \
  }                                             \
  bool operator<(const This &o) const {         \
    if (f0< o.f0) return true;                  \
    else if (f0 > o.f0) return false;           \
    else return f1 < o.f1;                      \
  }                                             \
  bool operator>(const This &o) const {         \
    if (f0 > o.f0) return true;                 \
    else if (f0 < o.f0) return false;           \
    else return f1 > o.f1;                      \
  }                                             \
  bool operator>=(const This &o) const {        \
    return operator>(o) || operator==(o);       \
  }                                             \
  bool operator<=(const This &o) const {        \
    return operator<(o) || operator==(o);       \
  }                                             \
  int compare(const This &o) const {            \
    if (operator<(o)) return -1;                \
    else if (operator>(o)) return 1;            \
    else return 0;                              \
  }                                             \
  IMP_SWIG_COMPARISONS

#define IMP_COMPARISONS_3(f0, f1, f2)                   \
  bool operator==(const This &o) const {                \
    return (f0== o.f0 && f1==o.f1 && f2 == o.f2);       \
  }                                                     \
  bool operator!=(const This &o) const {                \
    return (f0!= o.f0 || f1 != o.f1 || f2 != o.f2);     \
  }                                                     \
  bool operator<(const This &o) const {                 \
    if (f0< o.f0) return true;                          \
    else if (f0 > o.f0) return false;                   \
    if (f1< o.f1) return true;                          \
    else if (f1 > o.f1) return false;                   \
    else return f2 < o.f2;                              \
  }                                                     \
  bool operator>(const This &o) const {                 \
    if (f0 > o.f0) return true;                         \
    else if (f0 < o.f0) return false;                   \
    if (f1 > o.f1) return true;                         \
    else if (f1 < o.f1) return false;                   \
    else return f2 > o.f2;                              \
  }                                                     \
  bool operator>=(const This &o) const {                \
    return operator>(o) || operator==(o);               \
  }                                                     \
  bool operator<=(const This &o) const {                \
    return operator<(o) || operator==(o);               \
  }                                                     \
  int compare(const This &o) const {                    \
    if (operator<(o)) return -1;                        \
    else if (operator>(o)) return 1;                    \
    else return 0;                                      \
  }                                                     \
  IMP_SWIG_COMPARISONS
#endif


/** \name Swap helpers

    Use the swap_with member function to swap two objects. The two
    objects mustbe of the same type (Name) and define
    the method \c swap_with(). The number suffix is the number of template
    arguments, all of which must be of class type.
    @{
*/
#define IMP_SWAP(Name)                                  \
  inline void swap(Name &a, Name &b) {a.swap_with(b);}  \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


#define IMP_SWAP_1(Name)                                        \
  template <class A>                                            \
  inline void swap(Name<A> &a, Name<A> &b) {a.swap_with(b);}    \
  IMP_REQUIRE_SEMICOLON_NAMESPACE



#define IMP_SWAP_2(Name)                                \
  template <class A, class B>                           \
  inline void swap(Name<A,B> &a, Name<A,B> &b) {        \
    a.swap_with(b);                                     \
  }                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


#define IMP_SWAP_3(Name)                                \
  template <class A, class B, class C>                  \
  inline void swap(Name<A,B,C> &a, Name<A,B,C> &b) {    \
    a.swap_with(b);                                     \
  }                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

/** @} */


//! Swap two member variables assuming the other object is called o
/** Swap the member \c var_name of the two objects (this and o).
 */
#define IMP_SWAP_MEMBER(var_name)               \
  std::swap(var_name, o.var_name)



//! Use a copy_from method to create a copy constructor and operator=
/** This macro is there to aid with classes which require a custom
    copy constructor. It simply forwards \c operator= and the copy
    constructor to a method \c copy_from() which should do the copying.

    You should think very hard before implementing a class which
    requires a custom copy custructor as it is easy to get wrong
    and you can easily wrap most resources with RAII objects
    (\external{en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization,
    wikipedia entry}).
*/
#define IMP_COPY_CONSTRUCTOR(Name) Name(const Name &o){copy_from(o);} \
  IMP_NO_SWIG(Name& operator=(const Name &o) {copy_from(o); return *this;}) \
  IMP_REQUIRE_SEMICOLON_CLASS(copy)





#ifdef _MSC_VER
// VC doesn't understand friends properly
#define IMP_REF_COUNTED_DESTRUCTOR(Name)        \
  public:                                       \
  virtual ~Name(){}                             \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest)              \
  public:                                                          \
  virtual ~Name(){dest}                                            \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)


#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)     \
  public:                                               \
  virtual ~Name()


#elif defined(SWIG)
// SWIG doesn't do friends right either, but we don't care as much
#define IMP_REF_COUNTED_DESTRUCTOR(Name)                \
  public:                                               \
  virtual ~Name(){}                                     \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest)              \
  public:                                                          \
  virtual ~Name(){dest}                                            \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)


#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)     \
  public:                                               \
  virtual ~Name()


#elif defined(IMP_DOXYGEN)
/* The destructor is unprotected for SWIG since if it is protected
   SWIG does not wrap the Python proxy distruction and so does not
   dereference the ref counted pointer. SWIG also gets confused
   on template friends.
*/
//! Ref counted objects should have private destructors
/** This macro defines a private destructor and adds the appropriate
    friend methods so that the class can be used with ref counting.
    By defining a private destructor, you make it so that the object
    cannot be declared on the stack and so must be ref counted.

    \see IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR()
    \see IMP::RefCounted
*/
#define IMP_REF_COUNTED_DESTRUCTOR(Name)


/** Like IMP_REF_COUNTED_DESTRUCTOR(), but the destructor is only
    declared, not defined.
*/
#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)

#else
#define IMP_REF_COUNTED_DESTRUCTOR(Name)                                \
  protected:                                                            \
  template <class T, class E> friend class IMP::internal::RefStuff;     \
  virtual ~Name(){}                                                     \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)

#define IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, dest)                   \
  protected:                                                            \
  template <class T, class E> friend class IMP::internal::RefStuff;     \
  virtual ~Name(){dest}                                                 \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)


#define IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Name)                     \
  protected:                                                            \
  template <class T, class E> friend class IMP::internal::RefStuff;     \
  virtual ~Name();                                                      \
public:                                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(destructor)



#endif

#ifdef IMP_DOXYGEN
/** Implement operator[] for C++ and python. The index type is
    Index and the expression that returns the value is expr. If the
    bounds_check_expr is false, then a UsageException is thrown
    in C++ or and IndexException if called from python.
*/
#define IMP_BRACKET(Value, Index, bounds_check_expr, expr)       \
  const Value operator[](Index) const;                           \
  Value& operator[](Index);

/** Implement operator[] for C++ and python. The index type is
    Index and the expression that returns the value is expr.
    The value returned is not mutable. If the
    bounds_check_expr is false, then a UsageException is thrown
    in C++ or and IndexException if called from python.
*/
#define IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)   \
  const Value operator[](Index) const;                             \
  Value& operator[](Index);



#elif !defined(SWIG)
#define IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)        \
  const Value& operator[](Index i) const {                              \
    IMP_USAGE_CHECK((bounds_check_expr), "Index out of range: "<< i);   \
    expr;                                                               \
  }                                                                     \
  const Value& __getitem__(Index i) const {                             \
    if (!(bounds_check_expr)) {                                         \
      IMP_THROW("Bad index " << i, IndexException);                     \
    }                                                                   \
    expr;                                                               \
  }                                                                     \

#define IMP_BRACKET(Value, Index, bounds_check_expr, expr)              \
  Value& operator[](Index i)  {                                         \
    IMP_USAGE_CHECK((bounds_check_expr), "Index out of range: "<< i);   \
    expr;                                                               \
  }                                                                     \
  void __setitem__(Index i, const Value &v) {                           \
    operator[](i)=v;                                                    \
  }                                                                     \
  IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)



#else
#define IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)        \
  const Value& __getitem__(Index i) const {                             \
    if (!(bounds_check_expr)) {                                         \
      IMP_THROW("Bad index " << i, IndexException);                     \
    }                                                                   \
    expr;                                                               \
  }

#define IMP_BRACKET(Value, Index, bounds_check_expr, expr)              \
  void __setitem__(Index i, const Value &v) {                           \
    operator[](i)=v;                                                    \
  }                                                                     \
  IMP_CONST_BRACKET(Value, Index, bounds_check_expr, expr)

#endif


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
    file, should declare the key itself as a \c static member variable to
    avoid initializing the key if the decorator is not used.

    See IMP::Decorator for a more detailed description
    of decorators.

    \param[in] Name is the name of the decorator, such as XYZR
    \param[in] Parent The class name for the parent of this class,
    typically Decorator

    \see IMP_DECORATOR_WITH_TRAITS()
*/
#define IMP_DECORATOR(Name, Parent)                                     \
  public:                                                               \
  /* Should be private but SWIG accesses it through the comparison
     macros*/                                                           \
IMP_NO_DOXYGEN(typedef Name This);                                      \
IMP_NO_DOXYGEN(typedef Parent ParentDecorator);                         \
Name(): Parent(){}                                                     \
explicit Name(::IMP::Particle *p): Parent(p) {                          \
  IMP_INTERNAL_CHECK(particle_is_instance(p),                           \
                     "Particle missing required attributes for decorator " \
                     << #Name << *p << std::endl);                      \
}                                                                       \
static Name decorate_particle(::IMP::Particle *p) {                     \
  IMP_CHECK_OBJECT(p);                                                  \
  if (!particle_is_instance(p)) {                                       \
    return Name();                                                      \
  }                                                                     \
  return Name(p);                                                       \
}                                                                       \
IMP_SHOWABLE(Name)


//! Define the basic things needed by a Decorator which has a traits object.
/** This macro is the same as IMP_DECORATOR() except that an extra object
    of type TraitsType is passed after the particle to
    - IMP::Decorator::particle_is_instance()
    - IMP::Decorator::setup_particle()
    As in the IMP::core::XYZR or IMP::core::Hierarchy,
    this object can be used to parameterize the Decorator. The traits
    object is stored in the decorator and made accessible through
    the get_traits() method.
*/
#define IMP_DECORATOR_WITH_TRAITS(Name, Parent, TraitsType, traits_name, \
                             default_traits)                            \
  private:                                                              \
  TraitsType traits_;                                                   \
public:                                                                 \
 IMP_NO_DOXYGEN(typedef Name This;)                                     \
 IMP_NO_DOXYGEN(typedef Parent ParentDecorator);                        \
 Name(){}                                                               \
 Name(const TraitsType &tr):                                            \
   traits_(tr) {}                                                       \
 Name(::IMP::Particle *p,                                               \
      const TraitsType &tr=default_traits):                             \
   Parent(p), traits_(tr) {                                             \
   IMP_INTERNAL_CHECK(particle_is_instance(p, tr),                      \
                      "Particle missing required attributes "           \
                      << " for decorator "                              \
                      << #Name << *p << std::endl);                     \
 }                                                                      \
 static Name decorate_particle(::IMP::Particle *p,                      \
                               const TraitsType &tr=default_traits) {   \
   if (!particle_is_instance(p, tr)) return Name();                     \
   else return Name(p, tr);                                             \
 }                                                                      \
 IMP_SHOWABLE(Name);                                                    \
 const TraitsType &get_##traits_name() const {                          \
   return get_decorator_traits();                                       \
 }                                                                      \
 typedef Parent DecoratorTraitsBase;                                    \
 typedef TraitsType DecoratorTraits;                                    \
 const DecoratorTraits& get_decorator_traits() const {return traits_;}  \
 static const DecoratorTraits& get_default_decorator_traits() {         \
   static TraitsType dt= default_traits;                                \
   return dt;                                                           \
 }                                                                      \
 IMP_NO_DOXYGEN(typedef boost::true_type DecoratorHasTraits)


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
  do {                                                                  \
    if (get_particle()->has_attribute(AttributeKey)) {                  \
      Type VALUE =  get_particle()->get_value(AttributeKey);            \
      has_action;                                                       \
    } else {                                                            \
      not_has_action;                                                   \
    }                                                                   \
  } while (false)



//! Set an attribute, creating it if it does not already exist.
/** Another common pattern is to have an assumed value if the attribute
    is not there. Then, you sometimes need to set the value whether it
    is there or not.
    \see IMP_DECORATOR_GET()
    \see IMP_DECORATOR_GET_SET()
*/
#define IMP_DECORATOR_SET(AttributeKey, value)           \
  do {                                                   \
  if (get_particle()->has_attribute(AttributeKey)) {     \
    get_particle()->set_value(AttributeKey, value)  ;    \
  } else {                                               \
    get_particle()->add_attribute(AttributeKey, value);  \
  }                                                      \
  } while (false)


//! Define methods for getting and setting a particular simple field
/** This macro defines methods to get and set a particular attribute.

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
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_CLASS(getset##name)


//! Define methods for getting and setting an optional simple field.
/** See IMP_DECORATOR_GET_SET(). The difference is that here you can provide
    a default value to use if the decorator does not have the attribute.

    \param[in] name The lower case name of the attribute
    \param[in] AttributeKey The expression to get the required attribute key.
    \param[in] Type The type of the attribute (upper case).
    \param[in] ReturnType The type to return from the get.
    \param[in] default_value The value returned if the attribute is missing.
*/
#define IMP_DECORATOR_GET_SET_OPT(name, AttributeKey, Type,     \
                                  ReturnType, default_value)    \
  ReturnType get_##name() const {                               \
    IMP_DECORATOR_GET(AttributeKey, Type,                       \
                      return static_cast<ReturnType>(VALUE),    \
                      return default_value);                    \
  }                                                             \
  void set_##name(ReturnType t) {                               \
    IMP_DECORATOR_SET(AttributeKey, t);                         \
  }                                                             \
  IMP_REQUIRE_SEMICOLON_CLASS(getset_##name)


#ifdef IMP_DOXYGEN
//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
 */
#define IMP_DECORATORS(Name, PluralName, Parent)
#else
#define IMP_DECORATORS(Name, PluralName, Parent)                \
  typedef IMP::Decorators<Name, Parent> PluralName;             \
  typedef IMP::Decorators<Name, Parent##Temp> PluralName##Temp
#endif

#ifdef IMP_DOXYGEN
//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
 */
#define IMP_DECORATORS_WITH_TRAITS(Name, PluralName, Parent)

#else
#define IMP_DECORATORS_WITH_TRAITS(Name, PluralName, Parent)  \
  typedef IMP::Decorators<Name, Parent> PluralName;           \
  typedef IMP::Decorators<Name, Parent##Temp> PluralName##Temp

#endif

//! Create a decorator that computes some sort of summary info on a set
/** Examples include a centroid or a cover for a set of particles.

    \param[in] Name The name for the decorator
    \param[in] Parent the parent decorator type
    \param[in] Members the way to pass a set of particles in
*/
#define IMP_SUMMARY_DECORATOR_DECL(Name, Parent, Members)               \
  class IMPCOREEXPORT Name: public Parent {                             \
    IMP_CONSTRAINT_DECORATOR_DECL(Name);                                \
  public:                                                               \
    IMP_DECORATOR(Name, Parent);                                        \
    static Name setup_particle(Particle *p,                             \
                               const Members &members);                 \
    static Name setup_particle(Particle *p,                             \
                               Refiner *ref);                           \
    ~Name();                                                            \
    static bool particle_is_instance(Particle *p) {                     \
      return p->has_attribute(get_constraint_key());                    \
    }                                                                   \
    IMP_NO_DOXYGEN(typedef boost::false_type DecoratorHasTraits);       \
  private:                                                              \
    /* hide set methods*/                                               \
    void set_coordinates() {};                                          \
    void set_coordinates_are_optimized()const{}                         \
    void set_coordinate() const {}                                      \
    void set_radius()const{}                                            \
  };                                                                    \
  IMP_DECORATORS(Name, Name##s, Parent##s)


/** See IMP_SUMMARY_DECORATOR_DECL()
    \param[in] Name The name for the decorator
    \param[in] Parent the parent decorator type
    \param[in] Members the way to pass a set of particles in
    \param[in] create_modifier the statements to create the modifier
    which computes the summary info. It should be called mod.
*/
#define IMP_SUMMARY_DECORATOR_DEF(Name, Parent, Members, create_modifier) \
  Name Name::setup_particle(Particle *p, const Members &ps) {           \
    Refiner *ref=new FixedRefiner(ps);                                  \
    create_modifier;                                                    \
    if (!Parent::particle_is_instance(p)) Parent::setup_particle(p);    \
    set_constraint(mod, new DerivativesToRefined(ref), p);              \
    return Name(p);                                                     \
  }                                                                     \
  Name Name::setup_particle(Particle *p, Refiner *ref) {                \
    create_modifier;                                                    \
    if (!Parent::particle_is_instance(p)) Parent::setup_particle(p);    \
    set_constraint(mod, new DerivativesToRefined(ref), p);              \
    return Name(p);                                                     \
  }                                                                     \
  Name::~Name(){}                                                       \
  IMP_NO_DOXYGEN(void Name::show(std::ostream &out) const {             \
      out << #Name << " at " << static_cast<Parent>(*this);             \
    })                                                                  \
  IMP_CONSTRAINT_DECORATOR_DEF(Name)



//! Define a set of attributes which form an array
/** The macro defines a set of functions for using the array:
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
    \param[in] instance_traits the traits object to use to manipulate things.
    This should inherit from or implement the interface of
    internal::ArrayOnAttributesHelper
    \param[in] ExternalType The name of the type to wrap the return type with.
    \param[in] ExternalTypes A vector of the return type.
*/
#define IMP_DECORATOR_ARRAY_DECL(protection, Class,                     \
                                 Name, name, plural,                    \
                                 instance_traits,                       \
                                 ExternalType, ExternalTypes)           \
  private:                                                              \
  template <class T>                                                    \
  static bool has_required_attributes_for_##name(Particle *p,           \
                                                 const T &traits) {     \
    return traits.has_required_attributes(p);                           \
  }                                                                     \
  template <class T>                                                    \
  static void add_required_attributes_for_##name(Particle *p,           \
                                                 const T &traits) {     \
    return traits.add_required_attributes(p);                           \
  }                                                                     \
  struct Name##AttrArrayAccessor {                                      \
    const Class *d_;                                                    \
    Name##AttrArrayAccessor(const Class *d): d_(d){}                    \
    Name##AttrArrayAccessor(): d_(NULL){}                               \
    typedef ExternalType result_type;                                   \
    result_type operator()(unsigned int i) const {                      \
      return d_->get_##name(i);                                         \
    }                                                                   \
    bool operator==(const Name##AttrArrayAccessor &o) const {           \
      return d_== o.d_;                                                 \
    }                                                                   \
  };                                                                    \
protection:                                                             \
 IMP_NO_SWIG(typedef IMP::internal                                      \
             ::IndexingIterator<Name##AttrArrayAccessor>                \
             Name##Iterator;)                                           \
 IMP_NO_SWIG(Name##Iterator plural##_begin() const {                    \
     return Name##Iterator(Name##AttrArrayAccessor(this));              \
   }                                                                    \
   Name##Iterator plural##_end() const {                                \
     return Name##Iterator(Name##AttrArrayAccessor(this),               \
                           get_number_of_##plural());                   \
   })                                                                   \
 ExternalType get_##name(unsigned int i) const {                        \
   return instance_traits.wrap(instance_traits.get_value(get_particle(), i)); \
 }                                                                      \
 unsigned int get_number_of_##plural() const {                          \
   return instance_traits.get_size(get_particle());                     \
 }                                                                      \
 unsigned int add_##name(ExternalType t) {                              \
   instance_traits.audit_value(t);                                      \
   unsigned int i= instance_traits.push_back(get_particle(),            \
                                             instance_traits.get_value(t)); \
   instance_traits.on_add(get_particle(), t, i);                        \
   return i;                                                            \
 }                                                                      \
 void add_##name##_at(ExternalType t, unsigned int idx) {               \
   instance_traits.audit_value(t);                                      \
   instance_traits.insert(get_particle(),                               \
                          idx,                                          \
                          instance_traits.get_value(t));                \
   instance_traits.on_add(get_particle(), t, idx);                      \
   for (unsigned int i= idx+1; i < get_number_of_##plural(); ++i) {     \
     instance_traits.on_change(get_particle(),                          \
                               instance_traits.get_value( get_particle(), i), \
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
   instance_traits.audit_value(t);                                      \
   unsigned int idx= instance_traits.get_index(get_particle(), t);      \
   instance_traits.on_remove(get_particle(), t);                        \
   instance_traits.erase(get_particle(),                                \
                idx);                                                   \
   for (unsigned int i= idx; i < get_number_of_##plural(); ++i) {       \
     instance_traits.on_change(get_particle(),                          \
                               instance_traits.get_value(get_particle(), i), \
                               i+1, i);                                 \
   }                                                                    \
 }                                                                      \
 void clear_##plural() {                                                \
   for (unsigned int i=0; i< get_number_of_##plural(); ++i) {           \
     instance_traits.on_remove(get_particle(), get_##name(i));          \
   }                                                                    \
   instance_traits.clear(get_particle());                               \
 }                                                                      \
 IMP_REQUIRE_SEMICOLON_CLASS(array##Name)

//! @}



#ifdef IMP_DOXYGEN
//! Define the type for storing sets of values
/** The macro defines the type Names. PluralName should be
    Names unless the English spelling is
    different.

    See
    \ref values "Value and Objects" for a description of what
    it means to be an object vs a value in \imp.
 */
#define IMP_VALUES(Name, PluralName)
#else
#define IMP_VALUES(Name, PluralName)            \
  typedef std::vector<Name> PluralName
#endif

#ifdef IMP_DOXYGEN
/** \name Showable
    Declare the methods needed by an object that can be printed,
    both from C++ and Python. Each value-type class should have an
    IMP_SHOWABLE() call internal to it and an IMP_OUTPUT_OPERATOR()
    call external to it.

    The suffixs are the number of template arguments that the
    object has (eg _1 means one template argument). _D means
    one integer template argument.
    @{
*/

/** This macro declares the method
    - void show(std::ostream &out) const
    It also makes it so that the object can be printed
    in Python.

    The \c ostream and \c sstream headers must be included.

    See also IMP_SHOWABLE_INLINE().

    Do not use with IMP::Object objects as they have their
    own show mechanism.
*/
#define IMP_SHOWABLE(Name)

//! Declare the methods needed by an object that can be printed
/** This macro declares the method
    - \c void \c show(std::ostream &out) const
    It also makes it so that the object can be printed
    in Python.

    The \c ostream and \c sstream headers must be included.

    See also IMP_SHOWABLE_INLINE()
*/
#define IMP_SHOWABLE_INLINE(Name, how_to_show)


//! Implement operator<< on class name, assuming it has one template argument
/** \copydetails IMP_OUTPUT_OPERATOR
 */
#define IMP_OUTPUT_OPERATOR_1(name)
//! Implement operator<< on class name, assuming it has two template arguments
/** \copydetails IMP_OUTPUT_OPERATOR
 */
#define IMP_OUTPUT_OPERATOR_2(name)
//! Implement operator<< on class name
/** The class named should define the method
    \c void \c show(std::ostream&).
*/
#define IMP_OUTPUT_OPERATOR(name)

//! Implement operator<< on class name templated by the dimension
/** The class named should define the method
    \c void \c show(std::ostream&).
*/
#define IMP_OUTPUT_OPERATOR_D(name)

//! Implement a hash function for the class
#define IMP_HASHABLE_INLINE(name, hashret)
/** @} */


#else
#define IMP_SHOWABLE(Name)                              \
  void show(std::ostream &out=std::cout) const;         \
  std::string __str__() const {                         \
    std::ostringstream out;                             \
    show(out);                                          \
    return out.str();                                   \
  }                                                     \
  std::string __repr__() const {                        \
    return std::string(#Name) + "("+__str__()+")";      \
  }                                                     \
  IMP_REQUIRE_SEMICOLON_CLASS(showable)

#define IMP_SHOWABLE_INLINE(Name, how_to_show)          \
  void show(std::ostream &out=std::cout) const{         \
    how_to_show;                                        \
  }                                                     \
  std::string __str__() const {                         \
    std::ostringstream out;                             \
    show(out);                                          \
    return out.str();                                   \
  }                                                     \
  std::string __repr__() const {                        \
    return std::string(#Name) + "("+__str__()+")";      \
  }                                                     \
  IMP_REQUIRE_SEMICOLON_CLASS(showable)

#define IMP_HASHABLE_INLINE(name, hashret)\
  std::size_t __hash__() const {          \
    hashret;                              \
  }

#if !defined(SWIG)
#define IMP_OUTPUT_OPERATOR_1(name)                                     \
  template <class L>                                                    \
  inline std::ostream& operator<<(std::ostream &out, const name<L> &i)  \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }                                                                     \
  template <class L>                                                    \
  inline void show(std::ostream &out, const name<L, M> &i)              \
    i.show(out);                                                        \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#define IMP_OUTPUT_OPERATOR_2(name)                                     \
  template <class L, class M>                                           \
  inline std::ostream& operator<<(std::ostream &out, const name<L, M> &i) \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }                                                                     \
  template <class L, class M>                                           \
  inline void show(std::ostream &out, const name<L, M> &i)              \
    i.show(out);                                                        \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


#define IMP_OUTPUT_OPERATOR(name)                                       \
  inline std::ostream &operator<<(std::ostream &out, const name &i)     \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }                                                                     \
  inline void show(std::ostream &out, const name &i) {                  \
    i.show(out);                                                        \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


#define IMP_OUTPUT_OPERATOR_D(name)                                     \
  template <unsigned int D>                                             \
  inline std::ostream &operator<<(std::ostream &out, const name<D> &i)  \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }                                                                     \
  template <unsigned int D>                                             \
  void show(std::ostream &out, const name<D> &i) {                      \
    i.show(out);                                                        \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE
#else
#define IMP_OUTPUT_OPERATOR_1(name)
#define IMP_OUTPUT_OPERATOR_2(name)
#define IMP_OUTPUT_OPERATOR(name)
#define IMP_OUTPUT_OPERATOR_D(name)
#endif
#endif

/** @} */


#ifdef IMP_DOXYGEN
//! Define a graph object in \imp
/** The docs for the graph should appear before the macro
    invocation.
 */
#define IMP_GRAPH(Name, type, VertexName, EdgeName) typedef boost::graph Name
#elif defined(SWIG)
#define IMP_GRAPH(Name, type, VertexName, EdgeName)  class Name
#else
#define IMP_GRAPH(Name, type, VertexName, EdgeName)                \
  typedef boost::adjacency_list<boost::vecS, boost::vecS,               \
  boost::type##S,                                                       \
  boost::property<boost::vertex_name_t, VertexName>,                    \
  boost::property<boost::edge_name_t,                                   \
  EdgeName> > Name
#endif

/** \name Macros to aid with implementing object classes

    These macros are here to aid with implementing classes that
    inherit from the various abstract base classes in the kernel. Each
    \imp object class should use one of the IMP_OBJECT(), IMP_RESTRAINT()
    etc. macros in the body of its declaration as well as an
    IMP_OBJECTS() invocation on the namespace level (and an
    IMP_SWIG_OBJECTS() call in the corresponding swig file). See
    \ref values "Value and Objects" for a description of what
    it means to be an object vs a value in \imp.

    Each  IMP_OBJECT()/IMP_RESTRAINT()-style macro
    declares/defines the set of needed functions. The declared
    functions should be defined in the associated \c .cpp file. By
    using the macros, you ensure that your class gets the names of the
    functions correct and it makes it easier to update your class if
    the functions should change.

    All of the IMP_OBJECT()/IMP_RESTRAINT()-style macros define the
    following methods:
    - IMP::Object::get_version_info()
    - an empty virtual destructor

    In addition, they all declare:
    - IMP::Object::do_show()

    For all macros, the Name parameter is the name of the class being
    implemented and the version_info parameter is the IMP::VersionInfo
    to use (probably get_version_info()).

    @{
*/

//! Define the basic things needed by any Object
/** This defines
    - IMP::Object::get_version_info()
    - a private destructor
    and declares
    - IMP::Object::do_show()
*/
#define IMP_OBJECT(Name)                                                \
  public:                                                               \
  virtual std::string get_type_name() const {return #Name;}             \
  virtual ::IMP::VersionInfo get_version_info() const {                 \
    return get_module_version_info();                                   \
  }                                                                     \
  /** \brief For python, cast a generic Object to this type. Return None
      if object is not the right type.*/                                \
static Name* get_from(Object *o) {                                      \
    return object_cast<Name>(o);                                        \
  }                                                                     \
IMP_NO_DOXYGEN(virtual void do_show(std::ostream &out) const);          \
IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, IMP_OBJECT_LOG;)



//! Define the basic things needed by any Object
/** This defines
    - IMP::Object::get_version_info()
    - a private destructor
    and declares
    - IMP::Object::do_show()
*/
#define IMP_OBJECT_INLINE(Name, show, destructor)                       \
  public:                                                               \
  virtual std::string get_type_name() const {return #Name;}             \
  virtual ::IMP::VersionInfo get_version_info() const {                 \
    return get_module_version_info();                                   \
  }                                                                     \
  IMP_NO_DOXYGEN (virtual void do_show(std::ostream &out) const {       \
      show;                                                             \
    });                                                                 \
  IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, IMP_OBJECT_LOG; destructor;)



//! Define the basic things needed by any internal Object
/** \see IMP_OBJECT
    This version also defines IMP::Object::do_show()
*/
#define IMP_INTERNAL_OBJECT(Name)                               \
  public:                                                       \
  virtual ::IMP::VersionInfo get_version_info() const {         \
    return get_module_version_info();                           \
  }                                                             \
  virtual std::string get_type_name() const {                   \
    return #Name;                                               \
  }                                                             \
private:                                                        \
 virtual void do_show(std::ostream & =std::cout) const {        \
 }                                                              \
 IMP_REF_COUNTED_INLINE_DESTRUCTOR(Name, IMP_OBJECT_LOG;)


#ifdef IMP_DOXYGEN
//! Define the types for storing sets of objects
/** The macro defines the types PluralName and PluralNameTemp.
    PluralName should be Names unless the English spelling is
    different.
 */
#define IMP_OBJECTS(Name, PluralName)
#else
#define IMP_OBJECTS(Name, PluralName)                   \
  typedef IMP::VectorOfRefCounted<Name*> PluralName;    \
  typedef std::vector<Name*> PluralName##Temp
#endif

//! Define the basic things you need for a Restraint.
/** In addition to the methods declared and defined by IMP_OBJECT
    it declares
    - IMP::Restraint::unprotected_evaluate()
    - IMP::Restraint::get_input_containers()
    - IMP::Restraint::get_input_particles()

    It also defines
    - IMP::Restraint::get_is_incremental() to return 0
    - IMP::Restraint::incremental_evaluate() to throw an exception
*/
#define IMP_RESTRAINT(Name)                                             \
  virtual double unprotected_evaluate(DerivativeAccumulator *accum) const; \
  ContainersTemp get_input_containers() const;                          \
  ParticlesTemp get_input_particles() const;                            \
  IMP_OBJECT(Name)

//! Define the basic things you need for a Restraint.
/** In addition to the methods declared and defined by IMP_OBJECT, it declares
    - IMP::Restraint::unprotected_evaluate()
    - IMP::Restraint::unprotected_incremental_evaluate()
    - IMP::Restraint::get_input_containers()
    - IMP::Restraint::get_input_particles()

    and it defines
    - IMP::Restraint::get_is_incremental() to return true
*/
#define IMP_INCREMENTAL_RESTRAINT(Name)                                 \
  virtual double unprotected_evaluate(DerivativeAccumulator *accum) const; \
  virtual bool get_is_incremental() const {return true;}                \
  virtual double                                                        \
  unprotected_incremental_evaluate(DerivativeAccumulator *accum) const; \
  ContainersTemp get_input_containers() const;                          \
  ParticlesTemp get_input_particles() const;                            \
  IMP_OBJECT(Name)

//! Define the basic things you need for an Optimizer.
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::Optimizer::optimize()

    \relatesalso IMP::Optimizer
*/
#define IMP_OPTIMIZER(Name)                             \
  virtual Float optimize(unsigned int max_steps);       \
  IMP_OBJECT(Name)

//! Define the basic things you need for a Sampler.
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::Sampler::do_sample()

    \relatesalso IMP::Sampler
*/
#define IMP_SAMPLER(Name)                       \
  IMP_OBJECT(Name);                             \
protected:                                      \
 ConfigurationSet* do_sample() const



//! Define the basics needed for an OptimizerState
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::OptimizerState::update()
*/
#define IMP_OPTIMIZER_STATE(Name)               \
  virtual void update();                        \
  IMP_OBJECT(Name)


//! Define the basics needed for an OptimizerState which acts every n steps
/** In addition to the methods done by all the macros, it declares
    - do_update(unsigned int call_number) where step number
    is the number of the optimization step, and call_number is the number
    of the call to do_update.
    It also defines
    - void set_skip_steps(unsigned int)

    If you use this macro, you should also include IMP/internal/utility.h.
*/
#define IMP_PERIODIC_OPTIMIZER_STATE(Name)                              \
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
  IMP_OBJECT(Name);                                                     \
  private:                                                              \
  ::IMP::internal::Counter skip_steps_, call_number_, update_number_    \

//! Define the basics needed for a ScoreState
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::ScoreState::do_before_evaluate()
    - IMP::ScoreState::do_after_evaluate()
    - IMP::ScoreState::get_input_containers()
    - IMP::ScoreState::get_output_containers()
    - IMP::ScoreState::get_input_particles()
    - IMP::ScoreState::get_output_particles()
*/
#define IMP_SCORE_STATE(Name)                                   \
  protected:                                                    \
  virtual void do_before_evaluate();                            \
  virtual void do_after_evaluate(DerivativeAccumulator *da);    \
  virtual ContainersTemp get_input_containers() const;          \
  virtual ContainersTemp get_output_containers() const;         \
  virtual ParticlesTemp get_input_particles() const;            \
  virtual ParticlesTemp get_output_particles() const;           \
  IMP_OBJECT(Name)


//! Define the basics needed for a ScoreState
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::Constraint::do_update_attributes()
    - IMP::Constraint::do_update_derivatives()
*/
#define IMP_CONSTRAINT(Name)                                            \
  protected:                                                            \
  void do_update_attributes();                                          \
  void do_update_derivatives(DerivativeAccumulator *da);                \
  IMP_NO_DOXYGEN(void do_before_evaluate() { Name::do_update_attributes();}) \
  IMP_NO_DOXYGEN(void do_after_evaluate(DerivativeAccumulator*da) {     \
      if (da) Name::do_update_derivatives(da);})                        \
  virtual ContainersTemp get_input_containers() const;                  \
  virtual ContainersTemp get_output_containers() const;                 \
  virtual ParticlesTemp get_input_particles() const;                    \
  virtual ParticlesTemp get_output_particles() const;                   \
  IMP_OBJECT(Name)


//! Define the basics needed for a particle refiner
/** In addition to the methods done by all the macros, it declares
    - IMP::Refiner::get_can_refine()
    - IMP::Refiner::get_number_of_refined()
    - IMP::Refiner::get_refined()
    - IMP::Refiner::get_input_particles()
    \see IMP_SIMPLE_REFINER
*/
#define IMP_REFINER(Name)                                               \
  virtual bool get_can_refine(Particle*) const;                         \
  virtual Particle* get_refined(Particle *, unsigned int) const;        \
  virtual const ParticlesTemp get_refined(Particle *) const;            \
  virtual unsigned int get_number_of_refined(Particle *) const;         \
  virtual ParticlesTemp get_input_particles(Particle *p) const;         \
  virtual ContainersTemp get_input_containers(Particle *p) const;       \
  IMP_OBJECT(Name)


//! Define the basics needed for a particle refiner
/** In contrast to IMP_REFINER, if this macro is used, the
    Refiner::get_refined(Particle*) method is implemented using the
    other Refiner::get_refined() method and so does not have to be
    provided.

    \see IMP_REFINER
*/
#define IMP_SIMPLE_REFINER(Name)                                        \
  virtual bool get_can_refine(Particle*) const;                         \
  virtual Particle* get_refined(Particle *, unsigned int) const;        \
  virtual const ParticlesTemp get_refined(Particle *a) const {          \
    ParticlesTemp ret(get_number_of_refined(a));                        \
    for (unsigned int i=0; i< ret.size(); ++i) {                        \
      ret[i]= get_refined(a,i);                                         \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  virtual unsigned int get_number_of_refined(Particle *) const;         \
  virtual ParticlesTemp get_input_particles(Particle *p) const;         \
  virtual ContainersTemp get_input_containers(Particle *p) const;       \
  IMP_OBJECT(Name)


#ifndef IMP_DOXYGEN
#define IMP_SINGLETON_SCORE_BASE(Name)                                  \
  double evaluate(Particle *a, DerivativeAccumulator *da) const;        \
  double evaluate(const ParticlesTemp &ps,                              \
                  DerivativeAccumulator *da) const {                    \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate(ps[i], da);                                   \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  double evaluate_change(Particle *a,                                   \
                         DerivativeAccumulator *da) const {             \
    if (get_is_changed(a)) {                                            \
      DerivativeAccumulator nda;                                        \
      if (da) nda= DerivativeAccumulator(*da, -1);                      \
      double v= Name::evaluate(a->get_prechange_particle(),             \
                               da? &nda:NULL);                          \
      double rv= Name::evaluate(a, da)-v;                               \
      return rv;                                                        \
    } else {                                                            \
      return 0;                                                         \
    }                                                                   \
  }                                                                     \
  double evaluate_change(const ParticlesTemp &ps,                       \
                         DerivativeAccumulator *da) const {             \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate_change(ps[i], da);                            \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  double evaluate_prechange(Particle *a,                                \
                            DerivativeAccumulator *da) const {          \
    return Name::evaluate(a->get_prechange_particle(),                  \
                          da);                                          \
  }                                                                     \
  double evaluate_prechange(const ParticlesTemp &ps,                    \
                            DerivativeAccumulator *da) const {          \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate_prechange(ps[i], da);                         \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT(Name)
#endif


//! Declare the functions needed for a SingletonScore
/** In addition to the methods done by IMP_INTERACTON, it declares
    - IMP::SingletonScore::evaluate(IMP::Particle*,
    IMP::DerivativeAccumulator*)
    - IMP::SingletonScore::get_input_particles()
    - IMP::SingletonScore::get_is_changed()
    - IMP::SingletonScore::get_output_particles()

    The macro expects a class method
    - bool get_is_changed(Particle*) which returns true if the score
    needs to be recalculated.

    See IMP_SIMPLE_SINGLETON_SCORE() for a way of providing an
    implementation of that method.
*/
#define IMP_SINGLETON_SCORE(Name)                               \
  bool get_is_changed(Particle *p) const;                       \
  ParticlesTemp get_input_particles(Particle*) const;           \
  ContainersTemp get_input_containers(Particle *) const;        \
  IMP_SINGLETON_SCORE_BASE(Name)

//! Declare the functions needed for a SingletonScore
/** In addition to the methods declared and defined by IMP_SINGLETON_SCORE,
    the macro provides an implementation of
    - IMP::SingletonScore::get_is_changed()
    - IMP::SingletonScore::get_input_particles()
    - IMP::SingletonScore::get_input_containers()
    which assume that only the passed particle serves as input to the
    score.
*/
#define IMP_SIMPLE_SINGLETON_SCORE(Name)                        \
  bool get_is_changed(Particle *p) const {                      \
    return p->get_is_changed();                                 \
  }                                                             \
  ParticlesTemp get_input_particles(Particle*p) const {         \
    return ParticlesTemp(1,p);                                  \
  }                                                             \
  ContainersTemp get_input_containers(Particle *) const {       \
    return ContainersTemp();                                    \
  }                                                             \
  IMP_SINGLETON_SCORE_BASE(Name)


#ifndef IMP_DOXYGEN
#define IMP_PAIR_SCORE_BASE(Name)                                       \
  double evaluate(const ParticlePair &p,                                \
                  DerivativeAccumulator *da) const;                     \
  double evaluate(const ParticlePairsTemp &ps,                          \
                  DerivativeAccumulator *da) const {                    \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate(ps[i], da);                                   \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  double evaluate_change(const ParticlePair &p,                         \
                         DerivativeAccumulator *da) const {             \
    if (get_is_changed(p)){                                             \
      DerivativeAccumulator nda;                                        \
      if (da) nda= DerivativeAccumulator(*da, -1);                      \
      double v= Name::evaluate(ParticlePair(p[0]->get_prechange_particle(), \
                                            p[1]->get_prechange_particle()), \
                               da? &nda:NULL);                          \
      double rv= Name::evaluate(p, da)-v;                               \
      return rv;                                                        \
    } else {                                                            \
      return 0;                                                         \
    }                                                                   \
  }                                                                     \
  double evaluate_change(const ParticlePairsTemp &ps,                   \
                         DerivativeAccumulator *da) const {             \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate_change(ps[i], da);                            \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  double evaluate_prechange(const ParticlePair &p,                      \
                            DerivativeAccumulator *da) const {          \
    return Name::evaluate(ParticlePair(p[0]->get_prechange_particle(),  \
                                       p[1]->get_prechange_particle()), \
                          da);                                          \
  }                                                                     \
  double evaluate_prechange(const ParticlePairsTemp &ps,                \
                            DerivativeAccumulator *da) const {          \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate_prechange(ps[i], da);                         \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT(Name)
#endif

//! Declare the functions needed for a PairScore
/** In addition to the methods done by IMP_OBJECT(), it declares
    - IMP::PairScore::evaluate()
    - IMP::PairScore::get_input_particles()
    - IMP::PairScore::get_output_particles()
    - IMP::PairScore::get_is_changed()

    The macro expects a class method
    - bool get_is_changed(const ParticlePair&) which returns true if the
    score needs to be recalculated.

    See IMP_SIMPLE_PAIR_SCORE() for a way of providing an
    implementation of that method.
*/
#define IMP_PAIR_SCORE(Name)                                            \
  bool get_is_changed(const ParticlePair &pp) const;                    \
  ParticlesTemp get_input_particles(Particle *p) const;                 \
  ContainersTemp get_input_containers(Particle *p) const;     \
  IMP_PAIR_SCORE_BASE(Name)

//! Declare the functions needed for a SingletonScore
/** In addition to the methods declared and defined by IMP_PAIR_SCORE,
    the macro provides an implementation of
    - IMP::PairScore::get_is_changed()
    - IMP::PairScore::get_input_particles()
    - IMP::PairScore::get_input_containers()
    which assume that only the 2 passed particles serve as inputs to the
    score.
*/
#define IMP_SIMPLE_PAIR_SCORE(Name)                                     \
  bool get_is_changed(const ParticlePair &p) const {                    \
    return p[0]->get_is_changed() || p[1]->get_is_changed();            \
  }                                                                     \
  ParticlesTemp get_input_particles(Particle *p) const {                \
    ParticlesTemp r(1, p);                                              \
    return r;                                                           \
  }                                                                     \
  ContainersTemp get_input_containers(Particle *) const {    \
    return ContainersTemp();                                            \
  }                                                                     \
  IMP_PAIR_SCORE_BASE(Name)

#ifndef IMP_DOXYGEN
#define IMP_TRIPLET_SCORE_BASE(Name)                                    \
  double evaluate(const ParticleTriplet &p,                             \
                  DerivativeAccumulator *da) const;                     \
  double evaluate(const ParticleTripletsTemp &ps,                       \
                  DerivativeAccumulator *da) const {                    \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate(ps[i],da);                                    \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  double evaluate_change(const ParticleTriplet &p,                      \
                         DerivativeAccumulator *da) const {             \
    if (get_is_changed(p)) {                                            \
      DerivativeAccumulator nda;                                        \
      if (da) nda= DerivativeAccumulator(*da, -1);                      \
      double v                                                          \
        = Name::evaluate(ParticleTriplet(p[0]->get_prechange_particle(), \
                                         p[1]->get_prechange_particle(), \
                                         p[2]->get_prechange_particle()),\
                               da? &nda:NULL);                          \
      double rv= Name::evaluate(p, da)-v;                               \
      return rv;                                                        \
    } else {                                                            \
      return 0;                                                         \
    }                                                                   \
  }                                                                     \
  double evaluate_change(const ParticleTripletsTemp &ps,                \
                         DerivativeAccumulator *da) const {             \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate_change(ps[i], da);                            \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  double evaluate_prechange(const ParticleTriplet &p,                   \
                            DerivativeAccumulator *da) const {          \
    return Name::evaluate(ParticleTriplet(p[0]->get_prechange_particle(), \
                                          p[1]->get_prechange_particle(), \
                                          p[1]->get_prechange_particle()), \
                          da);                                          \
  }                                                                     \
  double evaluate_prechange(const ParticleTripletsTemp &ps,             \
                            DerivativeAccumulator *da) const {          \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate_prechange(ps[i], da);                         \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT(Name)
#endif

//! Declare the functions needed for a TripletScore
/** In addition to the methods done by IMP_OBJECT(), it declares
    - IMP::TripletScore::evaluate()
    - IMP::TripletScore::get_is_changed()
    - IMP::TripletScore::get_input_particles()
    - IMP::TripletScore::get_output_particles()

    The macro expects a class method
    - bool get_is_changed(ParticleTriplet) which returns true if the
    score needs to be recalculated.

    See IMP_SIMPLE_TRIPLET_SCORE() for a way of providing an
    implementation of that method.
*/
#define IMP_TRIPLET_SCORE(Name)                                         \
  bool get_is_changed(const ParticleTriplet &p) const;                  \
  ParticlesTemp get_input_particles(Particle *p) const;                 \
  ContainersTemp get_input_containers(Particle *p) const;               \
  IMP_TRIPLET_SCORE_BASE(Name)


//! Declare the functions needed for a TripletScore
/** In addition to the methods declared and defined by IMP_TRIPLET_SCORE,
    the macro provides an implementation of
    - IMP::TripletScore::get_is_changed()
    - IMP::TripletScore::get_input_particles()
    - IMP::TripletScore::get_input_containers()
    which assume that only the 3 passed particles serve as inputs to the
    score.
*/
#define IMP_SIMPLE_TRIPLET_SCORE(Name)                                  \
  bool get_is_changed(const ParticleTriplet &p) const {                 \
    return p[0]->get_is_changed() || p[1]->get_is_changed()             \
      || p[1]->get_is_changed();                                        \
  }                                                                     \
  ParticlesTemp get_input_particles(Particle *p) const {                \
    ParticlesTemp r(1,p);                                               \
    return r;                                                           \
  }                                                                     \
  ContainersTemp get_input_containers(Particle *p) const {              \
    return ContainersTemp();                                            \
  }                                                                     \
  IMP_TRIPLET_SCORE_BASE(Name)

//! Declare the functions needed for a QuadScore
/** In addition to the methods done by IMP_OBJECT(), it declares
    - IMP::QuadScore::evaluate()
    - IMP::QuadScore::get_input_particles()
    - IMP::QuadScore::get_output_particles()

    The macro expects a class method
    - bool get_is_changed(ParticleQuad) which returns true if the
    score needs to be recalculated.
*/
#define IMP_QUAD_SCORE(Name)                                            \
  double evaluate(const ParticleQuad &p,                                \
                  DerivativeAccumulator *da) const;                     \
  double evaluate(Particle *a, Particle *b, Particle *c, Particle *d,   \
                  DerivativeAccumulator *da) const {                    \
    return evaluate(ParticleQuad(a,b,c,d), da);                         \
  }                                                                     \
  double evaluate(const ParticleQuadsTemp &ps,                          \
                  DerivativeAccumulator *da) const {                    \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate(ps[i], da);                                   \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  double evaluate_change(const ParticleQuad &p,                         \
                         DerivativeAccumulator *da) const {             \
    if (p[0]->get_is_changed() || p[1]->get_is_changed()                \
        || p[2]->get_is_changed() || p[3]->get_is_changed()) {          \
      DerivativeAccumulator nda;                                        \
      if (da) nda= DerivativeAccumulator(*da, -1);                      \
      double v= Name::evaluate(ParticleQuad(p[0]->get_prechange_particle(), \
                                            p[1]->get_prechange_particle(), \
                                            p[2]->get_prechange_particle(), \
                                            p[3]->get_prechange_particle()), \
                               da? &nda:NULL);                          \
      double rv= Name::evaluate(p, da)-v;                               \
      return rv;                                                        \
    } else {                                                            \
      return 0;                                                         \
    }                                                                   \
  }                                                                     \
  double evaluate_change(const ParticleQuadsTemp &ps,                   \
                         DerivativeAccumulator *da) const {             \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate_change(ps[i], da);                            \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  double evaluate_prechange(const ParticleQuad &p,                      \
                            DerivativeAccumulator *da) const {          \
    return Name::evaluate(ParticleQuad(p[0]->get_prechange_particle(),  \
                                       p[1]->get_prechange_particle(),  \
                                       p[2]->get_prechange_particle(),  \
                                       p[3]->get_prechange_particle()), \
                          da);                                          \
  }                                                                     \
  double evaluate_prechange(const ParticleQuadsTemp &ps,                \
                            DerivativeAccumulator *da) const {          \
    double ret=0;                                                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      ret+=Name::evaluate_prechange(ps[i], da);                         \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  bool get_is_changed(const ParticleQuad &p) const;                     \
  ParticlesTemp get_input_particles(Particle *p) const;                 \
  ContainersTemp get_input_containers(Particle *p) const;               \
  IMP_OBJECT(Name)



//! Declare the functions needed for a SingletonModifier
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::SingletonModifier::apply(IMP::Particle*)
    - IMP::SingletonModifier::get_input_particles()
    - IMP::SingletonModifier::get_output_particles()
    \see IMP_SINGLETON_MODIFIER_DA
*/
#define IMP_SINGLETON_MODIFIER(Name)                                    \
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
  ParticlesTemp get_input_particles(Particle*) const;                   \
  ParticlesTemp get_output_particles(Particle*) const;                  \
  ContainersTemp get_input_containers(Particle*) const;                 \
  ContainersTemp get_output_containers(Particle*) const;                \
  IMP_OBJECT(Name)



//! Declare the functions needed for a PairModifier
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::PairModifier::apply(IMP::Particle*,IMP::Particle*)
    - IMP::PairModifier::get_input_particles()
    - IMP::PairModifier::get_output_particles()
    \see IMP_PAIR_MODIFIER_DA
*/
#define IMP_PAIR_MODIFIER(Name)                                          \
  void apply(const ParticlePair &p) const;                              \
  void apply(const ParticlePair &p, DerivativeAccumulator&) const{      \
    apply(p);                                                           \
  }                                                                     \
  void apply(const ParticlePairsTemp &ps) const {                       \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i]);                                               \
    }                                                                   \
  }                                                                     \
  void apply(const ParticlePairsTemp &ps,                               \
             DerivativeAccumulator &) const {                           \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i]);                                               \
    }                                                                   \
  }                                                                     \
  ParticlesTemp get_input_particles(Particle *p) const;                 \
  ParticlesTemp get_output_particles(Particle *p) const;                \
  ContainersTemp get_input_containers(Particle *p) const;               \
  ContainersTemp get_output_containers(Particle *p) const;              \
  IMP_OBJECT(Name)


//! Declare the functions needed for a SingletonModifier
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::SingletonModifier::apply(IMP::Particle*,
    IMP::DerivativeAccumulator&)
    - IMP::SingletonModifier::get_input_particles()
    - IMP::SingletonModifier::get_output_particles()

    \see IMP_SINGLETON_MODIFIER
*/
#define IMP_SINGLETON_MODIFIER_DA(Name)                                 \
  void apply(Particle *a, DerivativeAccumulator &da) const;             \
  void apply(Particle *) const{                                         \
    IMP_LOG(VERBOSE, "This modifier requires a derivative accumulator " \
            << *this << std::endl);                                     \
  }                                                                     \
  void apply(const ParticlesTemp &) const {                             \
    IMP_LOG(VERBOSE, "This modifier requires a derivative accumulator " \
            << *this << std::endl);                                     \
  }                                                                     \
  void apply(const ParticlesTemp &ps,                                   \
             DerivativeAccumulator &da) const {                         \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i], da);                                           \
    }                                                                   \
  }                                                                     \
  ParticlesTemp get_input_particles(Particle*) const;                   \
  ParticlesTemp get_output_particles(Particle*) const;                  \
  ContainersTemp get_input_containers(Particle*) const;                 \
  ContainersTemp get_output_containers(Particle*) const;                \
  IMP_OBJECT(Name)


//! Add interaction methods to a SingletonModifer
/** This macro is designed to be used in conjunction with
    IMP_SINGLETON_MODIFIER or IMP_SINGLETON_MODIFIER_DA. It adds
    definitions for the methods:
    - IMP::SingletonModifier::get_input_particles()
    - IMP::SingletonModifier::get_output_particles()
    for a modifier which updates the passed particle based on the results
    of refinement.
*/
#define IMP_SINGLETON_MODIFIER_FROM_REFINED(Name, refiner)              \
  ParticlesTemp Name::get_input_particles(Particle *p) const {          \
    ParticlesTemp ret= refiner->get_refined(p);                         \
    ParticlesTemp ret1= refiner->get_input_particles(p);                \
    ret.insert(ret.end(), ret1.begin(), ret1.end());                    \
    return ret;                                                         \
  }                                                                     \
  ParticlesTemp Name::get_output_particles(Particle *p) const {         \
    ParticlesTemp ret(1,p);                                             \
    return ret;                                                         \
  }                                                                     \
  ContainersTemp Name::get_input_containers(Particle *p) const {        \
    return refiner->get_input_containers(p);                            \
  }                                                                     \
  ContainersTemp Name::get_output_containers(Particle *) const {       \
    return ContainersTemp();                                            \
  }                                                                     \
  IMP_NO_DOXYGEN(void Name::do_show(std::ostream &out) const {          \
    out <<"refiner " << *refiner << std::endl;                          \
    })                                                                  \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

//! Add interaction methods to a SingletonModifer
/** This macro is designed to be used in conjunction with
    IMP_SINGLETON_MODIFIER or IMP_SINGLETON_MODIFIER_DA. It adds
    definitions for the methods:
    - IMP::SingletonModifier::get_input_particles()
    - IMP::SingletonModifier::get_output_particles()
    - IMP::Object::do_show()
    for a modifier which updates the refined particles based on the one
    they are refined from.

    This macro should appear in a .cpp file.
*/
#define IMP_SINGLETON_MODIFIER_TO_REFINED(Name, refiner)                \
  ParticlesTemp Name::get_input_particles(Particle *p) const {          \
    ParticlesTemp ret= refiner->get_input_particles(p);                 \
    ParticlesTemp ret2= refiner->get_refined(p);                        \
    ret.push_back(p);                                                   \
    ret.insert(ret.end(), ret2.begin(), ret2.end());                    \
    return ret;                                                         \
  }                                                                     \
  ParticlesTemp Name::get_output_particles(Particle *p) const {         \
    ParticlesTemp ret= refiner->get_refined(p);                         \
    return ret;                                                         \
  }                                                                     \
  ContainersTemp Name::get_input_containers(Particle *p) const {        \
    return refiner->get_input_containers(p);                            \
  }                                                                     \
  ContainersTemp Name::get_output_containers(Particle *) const {       \
    return ContainersTemp();                                            \
  }                                                                     \
  IMP_NO_DOXYGEN(void Name::do_show(std::ostream &out) const {          \
    out << "refiner " << *refiner << std::endl;                         \
    })                                                                  \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

//! Declare the functions needed for a PairModifier
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::PairModifier::apply(Particle*,Particle*,DerivativeAccumulator&)
    - IMP::PairModifier::get_input_particles()
    - IMP::PairModifier::get_output_particles()
    \see IMP_PAIR_MODIFIER
*/
#define IMP_PAIR_MODIFIER_DA(Name)                                      \
  void apply(const ParticlePair &p, DerivativeAccumulator *da) const;   \
  void apply(const ParticlePair &p) const{                              \
    IMP_LOG(VERBOSE, "This modifier requires a derivative accumulator " \
            << *this << std::endl);                                     \
  }                                                                     \
  void apply(const ParticlePairsTemp &ps) const {                       \
    IMP_LOG(VERBOSE, "This modifier requires a derivative accumulator " \
            << *this << std::endl);                                     \
  }                                                                     \
  void apply(const ParticlePairsTemp &ps,                               \
             DerivativeAccumulator &da) const {                         \
    for (unsigned int i=0; i< ps.size(); ++i) {                         \
      Name::apply(ps[i], da);                                           \
    }                                                                   \
  }                                                                     \
  ParticlesTemp get_input_particles(Particle *p) const;                 \
  ParticlesTemp get_output_particles(Particle *p) const;                \
  ContainersTemp get_input_containers(Particle *p) const;               \
  ContainersTemp get_output_containers(Particle *p) const;              \
  IMP_OBJECT(Name)



//! Define a simple SingletonModifier
/** In addition to the methods done by IMP_OBJECT, it defines
    - IMP::SingletonModifier::apply(IMP::Particle*) to the provided value
    - IMP::SingletonModifier::get_input_particles() to return the particle
    - IMP::SingletonModifier::get_output_particles() to return the particle
    - IMP::Object::do_show()
    This macro should only be used to define types which are used
    internally in algorithms and data structures.
    \see IMP_SINGLETON_MODIFIER_DA
    \see IMP_SINGLETON_MODIFIER
*/
#define IMP_INTERNAL_SINGLETON_MODIFIER(Name,                           \
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
  ParticlesTemp get_input_particles(Particle*p) const {                 \
    return ParticlesTemp(1,p);                                          \
  }                                                                     \
  ParticlesTemp get_output_particles(Particle*p) const {                \
    return ParticlesTemp(1,p);                                          \
  }                                                                     \
  ContainersTemp get_input_containers(Particle*) const {                \
    return ContainersTemp();                                            \
  }                                                                     \
  ContainersTemp get_output_containers(Particle*) const {               \
    return ContainersTemp();                                            \
  }                                                                     \
  IMP_INTERNAL_OBJECT(Name)


#ifndef IMP_DOXYGEN
#define IMP_IMPLEMENT_CONTAINER(Name, Tuple, PassValue)                 \
  template <class SM>                                                   \
  void template_apply(const SM *sm,                                     \
                      DerivativeAccumulator &da) {                      \
    apply_to_contents(boost::bind(static_cast<void (Tuple##Modifier::*) \
                        (PassValue,DerivativeAccumulator&) const>       \
                        (&Tuple##Modifier::apply), sm, _1, da));        \
  }                                                                     \
  template <class SS>                                                   \
  double template_evaluate(const SS *s,                                 \
                           DerivativeAccumulator *da) const {           \
    return accumulate_over_contents(boost::bind(static_cast<double      \
                                                (Tuple##Score::*)       \
                              (PassValue,DerivativeAccumulator*) const> \
                                (&Tuple##Score::evaluate), s, _1, da)); \
  }                                                                     \
  template <class SS>                                                   \
  double template_evaluate_change(const SS *s,                          \
                                  DerivativeAccumulator *da) const {    \
    return accumulate_over_contents(boost::bind(static_cast<double      \
                                                (Tuple##Score::*)       \
                              (PassValue,DerivativeAccumulator*) const> \
                         (&Tuple##Score::evaluate_change), s, _1, da)); \
  }                                                                     \
  template <class SS>                                                   \
  double template_evaluate_prechange(const SS *s,                       \
                                     DerivativeAccumulator *da) const { \
    return accumulate_over_contents(boost::bind(static_cast<double      \
                                                (Tuple##Score::*)       \
                              (PassValue,DerivativeAccumulator*) const> \
                      (&Tuple##Score::evaluate_prechange), s, _1, da)); \
  }                                                                     \
  void apply(const Tuple##Modifier *sm) {                               \
    template_apply(sm);                                                 \
  }                                                                     \
  void apply(const Tuple##Modifier *sm,                                 \
             DerivativeAccumulator &da) {                               \
    template_apply(sm, da);                                             \
  }                                                                     \
  double evaluate(const Tuple##Score *s,                                \
                  DerivativeAccumulator *da) const {                    \
    return template_evaluate(s, da);                                    \
  }                                                                     \
  double evaluate_change(const Tuple##Score *s,                         \
                         DerivativeAccumulator *da) const {             \
    return template_evaluate_change(s, da);                             \
  }                                                                     \
  double evaluate_prechange(const Tuple##Score *s,                      \
                            DerivativeAccumulator *da) const {          \
    return template_evaluate_prechange(s, da);                          \
  }                                                                     \
  template <class SM>                                                   \
  void template_apply(const SM *sm) {                                   \
    apply_to_contents(boost::bind(static_cast<void (Tuple##Modifier::*) \
                        (PassValue) const>(&Tuple##Modifier::apply),    \
                        sm, _1));                                       \
  }                                                                     \
  ParticlesTemp get_contained_particles() const;                        \
  bool get_contained_particles_changed() const;                         \
  IMP_OBJECT(Name)
#endif


//! Declare the needed functions for a SingletonContainer
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::SingletonContainer::get_contains_particle()
    - IMP::SingletonContainer::get_number_of_particles()
    - IMP::SingletonContainer::get_particle()
    - IMP::SingletonContainer::apply()
    - IMP::SingletonContainer::evaluate()
    - IMP::Interaction::get_input_objects()
*/
#define IMP_SINGLETON_CONTAINER(Name)                                   \
  SingletonContainerPair get_added_and_removed_containers() const;      \
  bool get_contains_particle(Particle* p) const;                        \
  unsigned int get_number_of_particles() const;                         \
  Particle *get_particle(unsigned int i) const;                         \
  IMP_IMPLEMENT_CONTAINER(Name, Singleton, Particle*)


//! Declare the needed functions for a PairContainer
/** In addition to the methods of IMP_OBJECT, it declares
    - IMP::PairContainer::get_contains_particle_pair()
    - IMP::PairContainer::get_number_of_particle_pairs()
    - IMP::PairContainer::get_particle_pair()
    - IMP::PairContainer::apply()
    - IMP::PairContainer::evaluate()
    - IMP::Interaction::get_input_objects()
*/
#define IMP_PAIR_CONTAINER(Name)                                        \
  PairContainerPair get_added_and_removed_containers() const;           \
  bool get_contains_particle_pair(const ParticlePair &p) const;         \
  unsigned int get_number_of_particle_pairs() const;                    \
  ParticlePair get_particle_pair(unsigned int i) const;                 \
  IMP_IMPLEMENT_CONTAINER(Name, Pair, const ParticlePair&)



//! Declare the needed functions for a TripletContainer
/** In addition to the methods of IMP_OBJECT, it declares
    - IMP::TripletContainer::get_contains_particle_triplet()
    - IMP::TripletContainer::get_number_of_particle_triplets()
    - IMP::TripletContainer::get_particle_triplet()
    - IMP::TripletContainer::apply()
    - IMP::TripletContainer::evaluate()
    - IMP::Interaction::get_input_objects()
*/
#define IMP_TRIPLET_CONTAINER(Name)                                     \
  TripletContainerPair get_added_and_removed_containers() const;        \
  bool get_contains_particle_triplet(const ParticleTriplet &p) const;   \
  unsigned int get_number_of_particle_triplets() const;                 \
  ParticleTriplet get_particle_triplet(unsigned int i) const;           \
  IMP_IMPLEMENT_CONTAINER(Name, Triplet, const ParticleTriplet&)



//! Declare the needed functions for a QuadContainer
/** In addition to the methods of IMP_OBJECT, it declares
    - IMP::QuadContainer::get_contains_particle_quad()
    - IMP::QuadContainer::get_number_of_particle_quads()
    - IMP::QuadContainer::get_particle_quad()
    - IMP::QuadContainer::apply()
    - IMP::QuadContainer::evaluate()
    - IMP::Interaction::get_input_objects()
*/
#define IMP_QUAD_CONTAINER(Name)                                \
  QuadContainerPair get_added_and_removed_containers() const;   \
  bool get_contains_particle_quad(const ParticleQuad &p) const; \
  unsigned int get_number_of_particle_quads() const;            \
  ParticleQuad get_particle_quad(unsigned int i) const;         \
  IMP_IMPLEMENT_CONTAINER(Name, Quad, const ParticleQuad&)



#ifndef SWIG
//! Declare the needed functions for a SingletonFilter
/** In addition to the methods done by all the macros, it declares
    - IMP::SingletonFilter::get_contains_particle()
    - IMP::SingletonFilter::get_input_particles()
*/
#define IMP_SINGLETON_FILTER(Name)                                      \
  private:                                                              \
  struct GCP {                                                          \
    const Name *back_;                                                  \
    GCP(const Name *n): back_(n){}                                      \
    bool operator()(Particle* p) const {                                \
      return back_->Name::get_contains_particle(p);                     \
    }                                                                   \
  };                                                                    \
  public:                                                               \
  bool get_contains_particle(Particle* p) const;                        \
  ParticlesTemp get_input_particles(Particle*t) const;                  \
  ContainersTemp get_input_containers(Particle*t) const;                \
  void filter_in_place(ParticlesTemp &ps) const {                       \
    ps.erase(std::remove_if(ps.begin(), ps.end(),                       \
                            GCP(this)),                                 \
             ps.end());                                                 \
  }                                                                     \
  IMP_OBJECT(Name)


//! Declare the needed functions for a PairFilter
/** In addition to the methods done by all the macros, it declares
    - IMP::PairFilter::get_contains_particle_pair()
    - IMP::PairFilter::get_input_particles()
*/
#define IMP_PAIR_FILTER(Name)                                           \
  private:                                                              \
  struct GCP {                                                          \
    const Name *back_;                                                  \
    GCP(const Name *n): back_(n){}                                      \
    bool operator()(const ParticlePair &p) const {                      \
      return back_->Name::get_contains_particle_pair(p);                \
    }                                                                   \
  };                                                                    \
  public:                                                               \
  bool get_contains_particle_pair(const ParticlePair& p) const;         \
  ParticlesTemp get_input_particles(const ParticlePair& t) const;       \
  ContainersTemp get_input_containers(const ParticlePair& t) const;     \
  void filter_in_place(ParticlePairsTemp &ps) const {                   \
    ps.erase(std::remove_if(ps.begin(), ps.end(),                       \
                            GCP(this)),                                 \
             ps.end());                                                 \
  }                                                                     \
  IMP_OBJECT(Name)



//! Declare the needed functions for a TripletFilter
/** In addition to the methods done by all the macros, it declares
    - IMP::TripletFilter::get_contains_particle_triplet()
    - IMP::TripletFilter::get_input_particles()
*/
#define IMP_TRIPLET_FILTER(Name)                                        \
  private:                                                              \
  struct GCP {                                                          \
    const Name *back_;                                                  \
    GCP(const Name *n): back_(n){}                                      \
    bool operator()(const ParticleTriplet &p) const {                   \
      return back_->Name::get_contains_particle_triplet(p);             \
    }                                                                   \
  };                                                                    \
  bool get_contains_particle_triplet(const ParticleTriplet& p) const;   \
  ParticlesTemp get_input_particles(const ParticleTriplet& t) const;    \
  ContainersTemp get_input_containers(const ParticleTriplet& t) const;  \
  void filter_in_place(ParticleTripletsTemp &ps) const {                \
    ps.erase(std::remove_if(ps.begin(), ps.end(),                       \
                            GCP(this)),                                 \
             ps.end());                                                 \
  }                                                                     \
  IMP_OBJECT(Name)



//! Declare the needed functions for a QuadFilter
/** In addition to the methods done by all the macros, it declares
    - IMP::QuadFilter::get_contains_particle_quad()
    - IMP::QuadFilter::get_input_particles()
*/
#define IMP_QUAD_FILTER(Name)                                           \
  private:                                                              \
  struct GCP {                                                          \
    const Name *back_;                                                  \
    GCP(const Name *n): back_(n){}                                      \
    bool operator()(const ParticleQuad &p) const {                      \
      return back_->Name::get_contains_particle_quad(p);                \
    }                                                                   \
  };                                                                    \
  bool get_contains_particle_quad(const ParticleQuad& p) const;         \
  ParticlesTemp get_input_particles(const ParticleQuad& t) const;       \
  ContainersTemp get_input_containers(const ParticleQuad& t) const;     \
  void filter_in_place(ParticleQuadsTemp &ps) const {                   \
    ps.erase(std::remove_if(ps.begin(), ps.end(),                       \
                            GCP(this)),                                 \
             ps.end());                                                 \
  }                                                                     \
  IMP_OBJECT(Name)
#else
#define IMP_SINGLETON_FILTER(Name)                                      \
  bool get_contains_particle(Particle* p) const;                        \
  ParticlesTemp get_input_particles(Particle*t) const;                  \
  ContainersTemp get_input_containers(Particle*t) const;                \
  IMP_OBJECT(Name)


#define IMP_PAIR_FILTER(Name)                                           \
  bool get_contains_particle_pair(const ParticlePair& p) const;         \
  ParticlesTemp get_input_particles(const ParticlePair& t) const;       \
  ContainersTemp get_input_containers(const ParticlePair& t) const;     \
  IMP_OBJECT(Name)


#define IMP_TRIPLET_FILTER(Name)                                       \
  bool get_contains_particle_triplet(const ParticleTriplet& p) const;  \
  ParticlesTemp get_input_particles(const ParticleTriplet& t) const;   \
  ContainersTemp get_input_containers(const ParticleTriplet& t) const; \
  IMP_OBJECT(Name)



#define IMP_QUAD_FILTER(Name)                                           \
  bool get_contains_particle_quad(const ParticleQuad& p) const;         \
  ParticlesTemp get_input_particles(const ParticleQuad& t) const;       \
  ContainersTemp get_input_containers(const ParticleQuad& t) const;     \
  IMP_OBJECT(Name)

#endif


//! Declare the needed functions for a UnaryFunction
/** In addition to the methods done by all the macros, it declares
    - IMP::UnaryFunction::evaluate()
    - IMP::UnaryFunction::evaluate_with_derivatives()

    \see IMP_UNARY_FUNCTION_INLINE
*/
#define IMP_UNARY_FUNCTION(Name)                                        \
  virtual DerivativePair evaluate_with_derivative(double feature) const; \
  virtual double evaluate(double feature) const;                        \
  IMP_OBJECT(Name)



//! Declare the needed functions for a UnaryFunction which evaluates inline
/** This macro declares all the functions needed for an IMP::UnaryFunction
    inline in the class. There is no need for an associated \c .cpp file.

    The last three arguments are expressions that evaluate to the
    unary function value and derivative and are sent to the stream in the
    show function, respectively. The input to the function is called
    \c feature.

    \see IMP_UNARY_FUNCTION
*/
#define IMP_UNARY_FUNCTION_INLINE(Name, value_expression,               \
                                  derivative_expression, show_expression) \
  virtual DerivativePair evaluate_with_derivative(double feature) const { \
    return DerivativePair((value_expression), (derivative_expression)); \
  }                                                                     \
  virtual double evaluate(double feature) const {                       \
    return (value_expression);                                          \
  }                                                                     \
  IMP_OBJECT_INLINE(Name, out << show_expression, {})



//! Declare a IMP::FailureHandler
/** In addition to the standard methods it declares:
    - IMP::FailureHandler::handle_failure()
*/
#define IMP_FAILURE_HANDLER(Name)               \
  void handle_failure();                        \
  IMP_OBJECT(Name)


//! Define a pair of classes to handle saving the model
/** This macro defines two classes:
 - NameOptimizerState
 - NameFailureHandler
 to handling saving the model in the specified way during
 optimization and on failures, respectively.
 \param[in] Name The name to prefix the class names
 \param[in] args The parentesized arguments to the constructor. The
   last one should be a string called file_name.
 \param[in] vars The needed member variables.
 \param[in] constr The body of the constructor.
 \param[in] functs Any functions (declaration and definition) to
            go in the class bodies.
 \param[in] save_action The action to take to perform the save. The
            name to save to is know as file_name

 The headers "IMP/OptimizerState.h", "IMP/FailureHandler.h", "boost/format.hpp"
 and "IMP/internal/utility.h" must be included.
 */
#define IMP_MODEL_SAVE(Name, args, vars, constr, functs, save_action)   \
  class Name##OptimizerState: public OptimizerState {                   \
    ::IMP::internal::Counter skip_steps_, call_number_, update_number_; \
    std::string file_name_;                                             \
    vars                                                                \
    virtual void update() {                                             \
      if (call_number_%(skip_steps_+1) ==0) {                           \
        std::ostringstream oss;                                         \
        bool formatted=false;                                           \
        try {                                                           \
          oss << boost::format(file_name_) % update_number_;            \
          formatted=true;                                               \
        } catch(...){                                                   \
        }                                                               \
        if (formatted) {                                                \
          write(oss.str());                                             \
        } else {                                                        \
          write(file_name_);                                            \
        }                                                               \
        ++update_number_;                                               \
      }                                                                 \
      ++call_number_;                                                   \
    }                                                                   \
  public:                                                               \
/** Write to a file generated from the passed filename every
skip_steps steps. The file_name constructor argument should contain
"%1%" if you don't want to write the same file each time.
*/                                                                      \
    Name##OptimizerState args :                                         \
    OptimizerState(std::string("Writer to ")+file_name),                \
      file_name_(file_name) {constr}                                    \
    functs                                                              \
    void set_skip_steps(unsigned int k) {                               \
      skip_steps_=k;                                                    \
      call_number_=0;                                                   \
    }                                                                   \
    void write(std::string file_name) const {                           \
      save_action                                                       \
        }                                                               \
  private:                                                              \
    void do_update(unsigned int call_number);                           \
    IMP_OBJECT_INLINE(Name##OptimizerState,                             \
                      out << "Write to " << file_name_ << std::endl;,); \
  };                                                                    \
IMP_OBJECTS(Name##OptimizerState, Name##OptimizerStates);               \
/** Write to a file when a failure occurs.*/                            \
class Name##FailureHandler: public FailureHandler {                     \
  std::string file_name_;                                               \
  vars                                                                  \
  public:                                                               \
  Name##FailureHandler args :                                           \
  FailureHandler(std::string("Writer to ")+file_name),                  \
    file_name_(file_name) {                                             \
    constr}                                                             \
  functs                                                                \
  void handle_failure() {                                               \
    const std::string file_name=file_name_;                             \
    save_action                                                         \
      }                                                                 \
  IMP_OBJECT_INLINE(Name##FailureHandler,                               \
                    out << "Write to " << file_name_ << std::endl;,);   \
};                                                                      \
IMP_OBJECTS(Name##FailureHandler, Name##FailureHandlers);


//! Declare a RAII-style class
/** Since such classes are typically quite small and simple, all
    the implementation is inline. The macro declares
    - default constructor
    - RAII::set()
    - RAII::reset()
    - destructor
    The Set and Reset arguments are the code to run in the respective
    functions. "args" are the argument string (in parens) for the
    constructor and set.
*/
#define IMP_RAII(Name, args, Initialize, Set, Reset)    \
  Name() {Initialize;}                                  \
  explicit Name args {Initialize; Set}                  \
  void set args {reset();                               \
    Set;}                                               \
  void reset() {Reset;}                                 \
  ~Name () {reset();}                                   \
private:                                                \
 void operator=(const Name &){}                         \
 Name(const Name &){}                                   \
public:                                                 \
  IMP_REQUIRE_SEMICOLON_CLASS(raii)

//! @}

//! Declare a ref counted pointer to a new object
/** \param[in] Typename The namespace qualified type being declared
    \param[in] varname The name for the ref counted pointer
    \param[in] args The arguments to the constructor, or ()
    if there are none.
    Please read the documentation for IMP::Pointer before using.
*/
#define IMP_NEW(Typename, varname, args)        \
  IMP::Pointer<Typename> varname(new Typename args)


/** \name Perform an operation on a set of particles

    These macros avoid various inefficiencies.

    The macros take the name of the sequence and the operation to
    peform. The item in the sequence is called _1.
    Use it like
    \code
    IMP_FOREACH_PARTICLE(sc, std::cout << _1->get_name());
    \endcode
    @{
*/
#define IMP_FOREACH_SINGLETON(sequence, operation)                      \
  do {                                                                  \
    if (sequence->get_provides_access()) {                              \
      const ParticlesTemp &imp_foreach_access=sequence->get_access();   \
      for (unsigned int  _2=0; _2< imp_foreach_access.size(); ++_2) {   \
        IMP::Particle* _1= imp_foreach_access[_2];                      \
        bool imp_foreach_break=false;                                   \
        operation                                                       \
          if (imp_foreach_break)  break;                                \
      }                                                                 \
      IMP_INTERNAL_CHECK(imp_foreach_access.size()                      \
                         == sequence->get_number_of_particles(),        \
                         "Sizes do not match");                         \
    } else {                                                            \
      unsigned int imp_foreach_size= sequence->get_number_of_particles(); \
      for (unsigned int _2=0;                                           \
           _2 != imp_foreach_size;                                      \
           ++_2) {                                                      \
        IMP::Particle* _1= sequence->get_particle(_2);                  \
        bool imp_foreach_break=false;                                   \
        operation                                                       \
          if (imp_foreach_break) break;                                 \
      }                                                                 \
      }                                                                 \
  } while (false)

#define IMP_FOREACH_PAIR(sequence, operation)                           \
  do {                                                                  \
    if (sequence->get_provides_access()) {                              \
      const ParticlePairsTemp &imp_foreach_access=sequence->get_access(); \
      for (unsigned int _2=0; _2< imp_foreach_access.size(); ++_2) {    \
        IMP::ParticlePair _1= imp_foreach_access[_2];                   \
        bool imp_foreach_break=false;                                   \
        operation                                                       \
          if (imp_foreach_break) { break;}                              \
      }                                                                 \
    } else {                                                            \
      unsigned int imp_foreach_size= sequence->get_number_of_particle_pairs(); \
      for (unsigned int _2=0;                                           \
           _2 != imp_foreach_size;                                      \
           ++_2) {                                                      \
        IMP::ParticlePair _1= sequence->get_particle_pair(_2);          \
        bool imp_foreach_break=false;                                   \
        operation                                                       \
          if (imp_foreach_break) break;                                 \
      }                                                                 \
    }                                                                   \
  } while (false)

#define IMP_FOREACH_TRIPLET(sequence, operation)                        \
  do {                                                                  \
    unsigned int imp_foreach_size= IMP::internal::get_size(sequence);   \
    for (unsigned int _2=0;                                             \
         _2 != imp_foreach_size;                                        \
         ++_2) {                                                        \
      IMP::ParticleTriplet _1= IMP::internal::get(sequence,             \
                                                  _2);                  \
      bool imp_foreach_break=false;                                     \
      operation                                                         \
        if (imp_foreach_break) break;                                   \
    }                                                                   \
  } while (false)

#define IMP_FOREACH_QUAD(sequence, operation)                           \
  do {                                                                  \
    unsigned int imp_foreach_size= IMP::internal::get_size(sequence);   \
    for (unsigned int _2=0;                                             \
         _2 != imp_foreach_size;                                        \
         ++_2) {                                                        \
      IMP::ParticleQuad _1= IMP::internal::get(sequence,                \
                                               _2);                     \
      bool imp_foreach_break=false;                                     \
      operation                                                         \
        if (imp_foreach_break) break;                                   \
    }                                                                   \
  } while (false)
/** @} */

/** Define a new key type.

    It defines two public types: Name, which is an instantiation of KeyBase, and
    Names which is a vector of Name.

    \param[in] Name The name for the new type.
    \param[in] Tag A (hopefully) unique integer to define this key type.

    \note We define a new class rather than use a typedef since SWIG has a
    bug dealing with names that start with ::. A fix has been commited to SVN
    for SWIG.

    \note The name in the typedef would have to start with ::IMP so it
    could be used out of the IMP namespace.
*/
#define IMP_DECLARE_KEY_TYPE(Name, Tag)         \
  typedef Key<Tag, true> Name;                  \
  IMP_VALUES(Name, Name##s)


/** Define a new key non lazy type where new types have to be created
    explicitly.

    \see IMP_DECLARE_KEY_TYPE
*/
#define IMP_DECLARE_CONTROLLED_KEY_TYPE(Name, Tag)      \
  typedef Key<Tag, false> Name;                         \
  IMP_VALUES(Name, Name##s)

#ifndef SWIG
/** Report dependencies of the container Name. Add the line
    deps_(new DependenciesScoreState(this), model) to the constructor
    initializer list. The input_deps argument should add the input
    containers to a variable ret.
*/
#define IMP_CONTAINER_DEPENDENCIES(Name, input_deps)                    \
  class DependenciesScoreState: public ScoreState {                     \
    Name* back_;                                                        \
  public:                                                               \
  DependenciesScoreState(Name *n):                                      \
    ScoreState(n->get_name()+" dependencies"),                          \
    back_(n){}                                                          \
  ContainersTemp get_input_containers() const{                          \
    ContainersTemp ret;                                                 \
    input_deps                                                          \
    return ret;                                                         \
  }                                                                     \
  ContainersTemp get_output_containers() const{                         \
    return ContainersTemp(1, back_);                                    \
  }                                                                     \
  ParticlesTemp get_input_particles() const {                           \
    return ParticlesTemp();                                             \
  }                                                                     \
  ParticlesTemp get_output_particles() const{                           \
    return ParticlesTemp();                                             \
  }                                                                     \
  void do_before_evaluate() {}                                          \
  void do_after_evaluate(DerivativeAccumulator *) {}                    \
  IMP_OBJECT_INLINE(DependenciesScoreState, {if (0) out<<1;}, {});      \
  };                                                                    \
  friend class DependenciesScoreState;                                  \
  ScopedScoreState deps_

#else
#define IMP_CONTAINER_DEPENDENCIES(Name, input_deps)
#endif

#ifndef IMP_DOXYGEN
#ifdef __GNU__
//! Use this to label a function with no side effects
/** \advanced */
#define IMP_NO_SIDEEFFECTS __attribute__ ((pure))
//! Use this to make the compiler (possibly) warn if the result is not used
/** \advanced */
#define IMP_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
//! restrict means that a variable is not aliased with this function
#define IMP_RESTRICT __restrict__
#define IMP_WARN_PREPROCESS(message) #warning message
#else
#define IMP_NO_SIDEEFFECTS
#define IMP_WARN_UNUSED_RESULT
#define IMP_RESTRICT
#define IMP_WARN_PREPROCESS(message)
#endif

#endif

#endif  /* IMP_MACROS_H */
