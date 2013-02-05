/**
 *  \file IMP/kernel/decorator_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DECORATOR_MACROS_H
#define IMPKERNEL_DECORATOR_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/log_macros.h>
#include <IMP/base/showable_macros.h>
#include <IMP/base/warning_macros.h>

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
IMP_NO_DOXYGEN(typedef Parent ParentDecorator);                         \
Name(): Parent(){}                                                      \
Name(Model *m, ParticleIndex id): Parent(m, id) {                       \
  IMP_INTERNAL_CHECK(particle_is_instance(m->get_particle(id)),         \
                     "Particle " << m->get_particle(id)->get_name()     \
                     << " missing required attributes for decorator "   \
                     << #Name << "\n"                                   \
                     << base::ShowFull(m->get_particle(id)));          \
}                                                                       \
explicit Name(::IMP::kernel::Particle *p): Parent(p) {                  \
  IMP_INTERNAL_CHECK(particle_is_instance(p),                           \
                     "Particle " << p->get_name()                       \
                     << " missing required attributes for decorator "   \
                     << #Name << "\n" << base::ShowFull(p));            \
}                                                                       \
static Name decorate_particle(::IMP::kernel::Particle *p) {             \
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
 IMP_NO_DOXYGEN(typedef Parent ParentDecorator);                        \
 Name(){}                                                               \
 Name(Model *m, ParticleIndex id, const TraitsType &tr): Parent(m, id), \
                                                         traits_(tr) {  \
   IMP_INTERNAL_CHECK(particle_is_instance(m->get_particle(id), tr),    \
                     "Particle " << m->get_particle(id)->get_name()     \
                     << " missing required attributes for decorator "   \
                      << #Name << "\n" << Showable(m->get_particle(id))); \
}                                                                       \
 Name(const TraitsType &tr):                                            \
   traits_(tr) {}                                                       \
 explicit Name(::IMP::kernel::Particle *p,                              \
      const TraitsType &tr=default_traits):                             \
   Parent(p), traits_(tr) {                                             \
   IMP_INTERNAL_CHECK(particle_is_instance(p, tr),                      \
                      "Particle " << p->get_name()                      \
                      << " missing required attributes "                \
                      << " for decorator "                              \
                      << #Name << "\n" << Showable(p));                 \
 }                                                                      \
 static Name decorate_particle(::IMP::kernel::Particle *p,              \
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
    if (get_model()->get_has_attribute(AttributeKey, get_particle_index())) { \
      Type VALUE =  get_model()->get_attribute(AttributeKey,            \
                                               get_particle_index());   \
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
#define IMP_DECORATOR_SET(AttributeKey, value)                  \
  do {                                                          \
    if (get_model()->get_has_attribute(AttributeKey,            \
                                       get_particle_index())) { \
      get_model()->set_attribute(AttributeKey,                  \
                                 get_particle_index(),          \
                                 value);                        \
    } else {                                                    \
      get_model()->add_attribute(AttributeKey,                  \
                                 get_particle_index(),          \
                                 value);                        \
    }                                                           \
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
    return static_cast<ReturnType>(get_model()                          \
                                   ->get_attribute(AttributeKey,        \
                                                   get_particle_index())); \
  }                                                                     \
  void set_##name(ReturnType t) {                                       \
    get_model()->set_attribute(AttributeKey, get_particle_index(), t);  \
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
#define IMP_DECORATORS_DECL(Name, PluralName)
#define IMP_DECORATORS_DEF(Name, PluralName)


//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
 */
#define IMP_DECORATORS(Name, PluralName, Parent)

#else
#define IMP_DECORATORS_DECL(Name, PluralName)        \
  class Name;                                        \
  typedef IMP::base::Vector<Name> PluralName

#define IMP_DECORATORS_DEF(Name, PluralName)                    \
  /* needed so there is no ambiguity with operator->*/          \
  inline std::ostream &operator<<(std::ostream &out, Name n) {  \
    n.show(out);                                                \
    return out;                                                 \
  }                                                             \


#define IMP_DECORATORS(Name, PluralName, Parent)                \
  IMP_DECORATORS_DECL(Name, PluralName);                        \
  IMP_DECORATORS_DEF(Name, PluralName)

#endif

#ifdef IMP_DOXYGEN
//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
 */
#define IMP_DECORATORS_WITH_TRAITS(Name, PluralName, Parent)

#else
#define IMP_DECORATORS_WITH_TRAITS(Name, PluralName, Parent)    \
  /* needed so there is no ambiguity with operator->*/          \
  inline std::ostream &operator<<(std::ostream &out, Name n) {  \
    n.show(out);                                                \
    return out;                                                 \
  }                                                             \
  typedef IMP::base::Vector<Name> PluralName

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


#endif  /* IMPKERNEL_DECORATOR_MACROS_H */
