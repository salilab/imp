/**
 *  \file IMP/kernel/decorator_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DECORATOR_MACROS_H
#define IMPKERNEL_DECORATOR_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include "particle_index.h"
#include "Particle.h"
#include "Decorator.h"
#include <IMP/base/check_macros.h>
#include <IMP/base/log_macros.h>
#include <IMP/base/showable_macros.h>
#include <IMP/base/warning_macros.h>



/** Implement the needed methods for a decorator based on
    - setup_particle()
    - get_is_setup()
    methods that you provide.
*/
#define IMP_DECORATOR_METHODS(Name, Parent)                                               \
 public:                                                                                  \
  /* Should be private but SWIG accesses it through the                 \
     comparison                                                         \
     macros*/ IMP_NO_DOXYGEN( \
      typedef Parent ParentDecorator);                                                    \
  Name() : Parent() {}                                                                    \
  Name(::IMP::kernel::Model *m, ::IMP::kernel::ParticleIndex id)                          \
      : Parent(m, id) {                                                                   \
    IMP_INTERNAL_CHECK(                                                                   \
        get_is_setup(m, id),                                                              \
        "Particle " << m->get_particle_name(id)                                           \
                    << " missing required attributes for decorator "                      \
                    << #Name);                                                            \
  }                                                                                       \
  explicit Name(const IMP::kernel::ParticleAdaptor &d) : Parent(d) {                      \
    IMP_INTERNAL_CHECK(                                                                   \
        get_is_setup(d.get_model(), d.get_particle_index()),                              \
        "Particle " << d.get_model()->get_particle_name(                                  \
                           d.get_particle_index())                                        \
                    << " missing required attributes for decorator "                      \
                    << #Name);                                                            \
  }                                                                                       \
  /** \deprecated_at{2.1} Check explicitly instead. */                                    \
  static Name decorate_particle(::IMP::kernel::Particle *p) {                             \
    IMP_CHECK_OBJECT(p);                                                                  \
    if (!get_is_setup(p->get_model(), p->get_index())) {                                  \
      return Name();                                                                      \
    }                                                                                     \
    return Name(p);                                                                       \
  }                                                                                       \
  static bool get_is_setup(const IMP::kernel::ParticleAdaptor &p) {                       \
    return get_is_setup(p.get_model(), p.get_particle_index());                           \
  }                                                                                       \
  IMP_SHOWABLE(Name)



/** Implement the needed methods for a decorator based on
    - setup_particle()
    - get_is_setup()
    methods that you provide.
*/
#define IMP_DECORATOR_WITH_TRAITS_METHODS(Name, Parent, TraitsType,                        \
                                          traits_name, default_traits)                     \
 private:                                                                                  \
  TraitsType traits_;                                                                      \
                                                                                           \
 public:                                                                                   \
  typedef TraitsType DecoratorTraits;                                                      \
  const DecoratorTraits &get_decorator_traits() const { return traits_; }                  \
  static const DecoratorTraits &get_default_decorator_traits() {                           \
    static TraitsType dt = default_traits;                                                 \
    return dt;                                                                             \
  }                                                                                        \
  /* Should be private but SWIG accesses it through the                  \
    comparison                                                          \
    macros*/ IMP_NO_DOXYGEN( \
      typedef Parent ParentDecorator);                                                     \
  IMP_NO_DOXYGEN(typedef boost::true_type DecoratorHasTraits);                             \
  Name() : Parent() {}                                                                     \
  Name(::IMP::kernel::Model *m, ::IMP::kernel::ParticleIndex id,                           \
       const TraitsType &tr = default_traits)                                              \
      : Parent(m, id), traits_(tr) {                                                       \
    IMP_INTERNAL_CHECK(                                                                    \
        get_is_setup(m, id, tr),                                                           \
        "Particle " << m->get_particle_name(id)                                            \
                    << " missing required attributes for decorator "                       \
                    << #Name);                                                             \
  }                                                                                        \
  explicit Name(const IMP::kernel::ParticleAdaptor &d,                                     \
                const TraitsType &tr = default_traits)                                     \
      : Parent(d), traits_(tr) {                                                           \
    IMP_INTERNAL_CHECK(                                                                    \
        get_is_setup(d.get_model(), d.get_particle_index(), tr),                           \
        "Particle " << d.get_model()->get_particle_name(                                   \
                           d.get_particle_index())                                         \
                    << " missing required attributes for decorator "                       \
                    << #Name);                                                             \
  }                                                                                        \
  /** \deprecated_at{2.1} Check explicitly instead. */                                     \
  static Name decorate_particle(::IMP::kernel::Particle *p,                                \
                                const TraitsType &tr = default_traits) {                   \
    IMP_CHECK_OBJECT(p);                                                                   \
    if (!get_is_setup(p->get_model(), p->get_index(), tr)) {                               \
      return Name();                                                                       \
    }                                                                                      \
    return Name(p);                                                                        \
  }                                                                                        \
  static bool get_is_setup(const IMP::kernel::ParticleAdaptor &p,                          \
                           const TraitsType &tr = default_traits) {                        \
    return get_is_setup(p.get_model(), p.get_particle_index(), tr);                        \
  }                                                                                        \
  IMP_SHOWABLE(Name)

/** Decorators need to be able to be set up from Particles, ParticleIndexes
    and other Decorators. To help keep things uniform, we provide macros
    to declare the setup functions. These macros expect that an appropriate
    `do_setup_particle(kernel::Model *, ParticleIndex, args...)` function is
    defined. But any docs needed before the macro invocation.
*/
#define IMP_DECORATOR_SETUP_0(Name)                                    \
  /** Setup the particle so it can be used with this decorator. */     \
  static Name setup_particle(kernel::Model *m, ParticleIndex pi) {     \
    IMP_USAGE_CHECK(!get_is_setup(m, pi),                              \
                    "Particle " << m->get_particle_name(pi)            \
                                << " already set up as " << #Name);    \
    do_setup_particle(m, pi);                                          \
    return Name(m, pi);                                                \
  }                                                                    \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator) { \
    return setup_particle(decorator.get_model(),                       \
                          decorator.get_particle_index());             \
  }
/** \see IMP_DECORATOR_SETUP_0() */
#define IMP_DECORATOR_SETUP_1(Name, FirstArgumentType, first_argument_name) \
  /** Setup the particle so that it can be used with this decorator */  \
  static Name setup_particle(kernel::Model *m, ParticleIndex pi,        \
                             FirstArgumentType first_argument_name) {   \
    IMP_USAGE_CHECK(!get_is_setup(m, pi),                               \
                    "Particle " << m->get_particle_name(pi)             \
                    << " already set up as " << #Name);                 \
    do_setup_particle(m, pi, first_argument_name);                      \
    return Name(m, pi);                                                 \
  }                                                                     \
  /** \see setup_particle(m, pi, first_argument_name) */                \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator,    \
                             FirstArgumentType first_argument_name) {   \
    return setup_particle(decorator.get_model(),                        \
                          decorator.get_particle_index(),               \
                          first_argument_name);                         \
  }

/** \see IMP_DECORATOR_SETUP_0() */
#define IMP_DECORATOR_SETUP_2(Name, FirstArgumentType, first_argument_name,    \
                              SecondArgumentType, second_argument_name)        \
  /** Setup the particle so it can be used with this decorator. */             \
  static Name setup_particle(kernel::Model *m, ParticleIndex pi,               \
                             FirstArgumentType first_argument_name,            \
                             SecondArgumentType second_argument_name) {        \
    IMP_USAGE_CHECK(!get_is_setup(m, pi),                                      \
                    "Particle " << m->get_particle_name(pi)                    \
                                << " already set up as " << #Name);            \
    do_setup_particle(m, pi, first_argument_name, second_argument_name);       \
    return Name(m, pi);                                                        \
  }                                                                            \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator,           \
                             FirstArgumentType first_argument_name,            \
                             SecondArgumentType second_argument_name) {        \
    return setup_particle(decorator.get_model(),                               \
                          decorator.get_particle_index(), first_argument_name, \
                          second_argument_name);                               \
  }
/** \see IMP_DECORATOR_SETUP_0() */
#define IMP_DECORATOR_SETUP_3(Name, FirstArgumentType, first_argument_name,    \
                              SecondArgumentType, second_argument_name,        \
                              ThirdArgumentType, third_argument_name)          \
  /** Setup the particle so it can be used with this decorator. */             \
  static Name setup_particle(kernel::Model *m, ParticleIndex pi,               \
                             FirstArgumentType first_argument_name,            \
                             SecondArgumentType second_argument_name,          \
                             ThirdArgumentType third_argument_name) {          \
    IMP_USAGE_CHECK(!get_is_setup(m, pi),                                      \
                    "Particle " << m->get_particle_name(pi)                    \
                                << " already set up as " << #Name);            \
    do_setup_particle(m, pi, first_argument_name, second_argument_name,        \
                      third_argument_name);                                    \
    return Name(m, pi);                                                        \
  }                                                                            \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator,           \
                             FirstArgumentType first_argument_name,            \
                             SecondArgumentType second_argument_name,          \
                             ThirdArgumentType third_argument_name) {          \
    return setup_particle(decorator.get_model(),                               \
                          decorator.get_particle_index(), first_argument_name, \
                          second_argument_name, third_argument_name);          \
  }
/** \see IMP_DECORATOR_SETUP_0() */
#define IMP_DECORATOR_SETUP_4(Name, FirstArgumentType, first_argument_name,    \
                              SecondArgumentType, second_argument_name,        \
                              ThirdArgumentType, third_argument_name,          \
                              FourthArgumentType, fourth_argument_name)        \
  /** Setup the particle so it can be used with this decorator. */             \
  static Name setup_particle(kernel::Model *m, ParticleIndex pi,               \
                             FirstArgumentType first_argument_name,            \
                             SecondArgumentType second_argument_name,          \
                             ThirdArgumentType third_argument_name,            \
                             FourthArgumentType fourth_argument_name) {        \
    IMP_USAGE_CHECK(!get_is_setup(m, pi),                                      \
                    "Particle " << m->get_particle_name(pi)                    \
                                << " already set up as " << #Name);            \
    do_setup_particle(m, pi, first_argument_name, second_argument_name,        \
                      third_argument_name, fourth_argument_name);              \
    return Name(m, pi);                                                        \
  }                                                                            \
  static Name setup_particle(IMP::kernel::ParticleAdaptor decorator,           \
                             FirstArgumentType first_argument_name,            \
                             SecondArgumentType second_argument_name,          \
                             ThirdArgumentType third_argument_name,            \
                             FourthArgumentType fourth_argument_name) {        \
    return setup_particle(decorator.get_model(),                               \
                          decorator.get_particle_index(), first_argument_name, \
                          second_argument_name, third_argument_name,           \
                          fourth_argument_name);                               \
  }

/** Decorators need to be able to be set up from Particles, ParticleIndexes
    and other Decorators. To help keep things uniform, we provide macros
    to declare the setup functions. These macros expect that an appropriate
    `do_setup_particle(kernel::Model *, ParticleIndex, args...)` function is
    defined. But any docs needed before the macro invocation.
*/
#define IMP_DECORATOR_TRAITS_SETUP_0(Name)                         \
  /** Setup the particle so it can be used with this decorator. */ \
  static Name setup_particle(                                      \
      kernel::Model *m, ParticleIndex pi,                          \
      DecoratorTraits tr = get_default_decorator_traits()) {       \
    do_setup_particle(m, pi, tr);                                  \
    return Name(m, pi, tr);                                        \
  }                                                                \
  static Name setup_particle(                                      \
      IMP::kernel::ParticleAdaptor d,                              \
      DecoratorTraits tr = get_default_decorator_traits()) {       \
    do_setup_particle(d.get_model(), d.get_particle_index(), tr);  \
    return Name(d.get_model(), d.get_particle_index(), tr);        \
  }
/** \see IMP_DECORATOR_TRAITS_SETUP_0() */
#define IMP_DECORATOR_TRAITS_SETUP_1(Name, FirstArgumentType,                \
                                     first_argument_name)                    \
  static Name setup_particle(                                                \
      kernel::Model *m, ParticleIndex pi,                                    \
      FirstArgumentType first_argument_name,                                 \
      DecoratorTraits tr = get_default_decorator_traits()) {                 \
    do_setup_particle(m, pi, first_argument_name, tr);                       \
    return Name(m, pi, tr);                                                  \
  }                                                                          \
  static Name setup_particle(                                                \
      IMP::kernel::ParticleAdaptor d, FirstArgumentType first_argument_name, \
      DecoratorTraits tr = get_default_decorator_traits()) {                 \
    do_setup_particle(d.get_model(), d.get_particle_index(),                 \
                      first_argument_name, tr);                              \
    return Name(d.get_model(), d.get_particle_index(), tr);                  \
  }
/** \see IMP_DECORATOR_TRAITS_SETUP_0() */
#define IMP_DECORATOR_TRAITS_SETUP_2(Name, FirstArgumentType,                 \
                                     first_argument_name, SecondArgumentType, \
                                     second_argument_name)                    \
  static Name setup_particle(                                                 \
      kernel::Model *m, ParticleIndex pi,                                     \
      FirstArgumentType first_argument_name,                                  \
      SecondArgumentType second_argument_name,                                \
      DecoratorTraits tr = get_default_decorator_traits()) {                  \
    do_setup_particle(m, pi, first_argument_name, second_argument_name, tr);  \
    return Name(m, pi, tr);                                                   \
  }                                                                           \
  static Name setup_particle(                                                 \
      IMP::kernel::ParticleAdaptor d, FirstArgumentType first_argument_name,  \
      SecondArgumentType second_argument_name,                                \
      DecoratorTraits tr = get_default_decorator_traits()) {                  \
    do_setup_particle(d.get_model(), d.get_particle_index(),                  \
                      first_argument_name, second_argument_name, tr);         \
    return Name(d.get_model(), d.get_particle_index(), tr);                   \
  }

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
#define IMP_DECORATOR_GET(AttributeKey, Type, has_action, not_has_action)     \
  do {                                                                        \
    if (get_model()->get_has_attribute(AttributeKey, get_particle_index())) { \
      Type VALUE =                                                            \
          get_model()->get_attribute(AttributeKey, get_particle_index());     \
      has_action;                                                             \
    } else {                                                                  \
      not_has_action;                                                         \
    }                                                                         \
  } while (false)

//! Set an attribute, creating it if it does not already exist.
/** Another common pattern is to have an assumed value if the attribute
    is not there. Then, you sometimes need to set the value whether it
    is there or not.
    \see IMP_DECORATOR_GET()
    \see IMP_DECORATOR_GET_SET()
*/
#define IMP_DECORATOR_SET(AttributeKey, value)                                \
  do {                                                                        \
    if (get_model()->get_has_attribute(AttributeKey, get_particle_index())) { \
      get_model()->set_attribute(AttributeKey, get_particle_index(), value);  \
    } else {                                                                  \
      get_model()->add_attribute(AttributeKey, get_particle_index(), value);  \
    }                                                                         \
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
#define IMP_DECORATOR_GET_SET(name, AttributeKey, Type, ReturnType)      \
  ReturnType get_##name() const {                                        \
    return static_cast<ReturnType>(                                      \
        get_model()->get_attribute(AttributeKey, get_particle_index())); \
  }                                                                      \
  void set_##name(ReturnType t) {                                        \
    get_model()->set_attribute(AttributeKey, get_particle_index(), t);   \
  }                                                                      \
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
#define IMP_DECORATOR_GET_SET_OPT(name, AttributeKey, Type, ReturnType, \
                                  default_value)                        \
  ReturnType get_##name() const {                                       \
    IMP_DECORATOR_GET(AttributeKey, Type,                               \
                      return static_cast<ReturnType>(VALUE),            \
                      return default_value);                            \
  }                                                                     \
  void set_##name(ReturnType t) { IMP_DECORATOR_SET(AttributeKey, t); } \
  IMP_REQUIRE_SEMICOLON_CLASS(getset_##name)

#define IMP_DECORATORS_DECL(Name, PluralName) \
  class Name;                                 \
  typedef IMP::base::Vector<Name> PluralName

#ifndef IMP_DOXYGEN
#define IMP_DECORATORS_DEF(Name, PluralName)                   \
  /* needed so there is no ambiguity with operator->*/         \
  inline std::ostream &operator<<(std::ostream &out, Name n) { \
    n.show(out);                                               \
    return out;                                                \
  }
#else
#define IMP_DECORATORS_DEF(Name, PluralName)
#endif

//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
    Parent is unused and remains for backward compatability
 */
#define IMP_DECORATORS(Name, PluralName, Parent) \
  IMP_DECORATORS_DECL(Name, PluralName);         \
  IMP_DECORATORS_DEF(Name, PluralName)

//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
 */
#define IMP_DECORATORS_WITH_TRAITS(Name, PluralName, Parent)   \
  /* needed so there is no ambiguity with operator->*/         \
  inline std::ostream &operator<<(std::ostream &out, Name n) { \
    n.show(out);                                               \
    return out;                                                \
  }                                                            \
  typedef IMP::base::Vector<Name> PluralName


/**
   Declares Decorator methods that allows (privately) setting a constraint
   and publicly getting that constraint
 */
#define IMP_CONSTRAINT_DECORATOR_DECL(Name)                                \
 private:                                                                  \
  static ObjectKey get_constraint_key();                                   \
 /** set a constraint associated with this decorator that applies 'before' \
     and 'after' before and after evaluation. The constraint is added as \
     a model ScoreState. If before and after are Null, the constraint is \
     reset and removed from the model list of score states.             \
 */                                                                     \
 static void set_constraint(SingletonModifier* before,                  \
                              SingletonDerivativeModifier* after, Model* m, \
                              ParticleIndex pi);                        \
                                                                           \
 public:                                                                   \
  Constraint* get_constraint() const {                                     \
    return dynamic_cast<Constraint*>(                                      \
        get_particle()->get_value(get_constraint_key()));                  \
  }                                                                        \
  IMP_REQUIRE_SEMICOLON_CLASS(constraint)

/**
   Defines Decorator methods that allows (privately) setting a constraint
   and publicly getting that constraint. The constraint is added as a
   score state to the model.
 */
#define IMP_CONSTRAINT_DECORATOR_DEF(Name)                                \
  ObjectKey Name::get_constraint_key() {                                  \
    static ObjectKey ret(#Name " score state");                           \
    return ret;                                                           \
  }                                                                       \
  void Name::set_constraint(SingletonModifier* before,                    \
                            SingletonDerivativeModifier* after, Model* m, \
                            ParticleIndex pi) {                           \
    if (!after && !before) {                                              \
      if (m->get_has_attribute(get_constraint_key(), pi)) {               \
        m->remove_score_state(dynamic_cast<ScoreState*>(                  \
            m->get_attribute(get_constraint_key(), pi)));                 \
        m->remove_attribute(get_constraint_key(), pi);                    \
      }                                                                   \
    } else {                                                            \
      Constraint* ss = new core::SingletonConstraint(                     \
          before, after, m, pi,                                           \
          std::string(#Name "updater for ") + m->get_particle_name(pi));  \
      m->add_attribute(get_constraint_key(), pi, ss);                     \
      m->add_score_state(ss);                                             \
    }                                                                     \
  }                                                                       \
  IMP_REQUIRE_SEMICOLON_NAMESPACE


/** Register a function that can be used to check that the particle
    is valid with respect to the decorator. The function should take
    a Particle* as an argument and return a bool. It should throw
    an exception if something is wrong.

    This macro should only be used in a .cpp file.
*/
#define IMP_CHECK_DECORATOR(Name, function) \
  IMP::kernel::internal::ParticleCheck Name##pc(Name::get_is_setup, function);


//! Create a decorator that computes some sort of summary info on a set
/** Examples include a centroid or a cover for a set of particles.

    \param[in] Name The name for the decorator
    \param[in] Parent the parent decorator type
    \param[in] Members the way to pass a set of particles in
    \param[in] SetupDoc extra documentation for setup
*/
#define IMP_SUMMARIZE_DECORATOR_DECL(Name, Parent, Members, SetupDoc) \
  class IMPCOREEXPORT Name : public Parent {                          \
    IMP_CONSTRAINT_DECORATOR_DECL(Name);                              \
  private:                                                            \
    /** Sets up Name over particles in pis */                         \
    static void do_setup_particle(kernel::Model *m, ParticleIndex pi, \
                                  const ParticleIndexes &pis);        \
    /** Sets up Name over particles passed by applying the refiner      \
        over the particle pi                                            \
    */                                                                  \
    static void do_setup_particle(kernel::Model *m, ParticleIndex pi,   \
                                  Refiner *ref);                      \
                                                                      \
   public:                                                            \
    IMP_DECORATOR_METHODS(Name, Parent);                              \
    /** Sets up Name over members, and constrains Name to be          \
        computed before model evaluation and to propogate derivatives \
        folllowing model evaulation.                                  \
        SetupDoc                                                      \
    */                                                                \
    IMP_DECORATOR_SETUP_1(Name, ParticleIndexesAdaptor, members);     \
    /** Sets up Name over particles passed by applying the refiner      \
        over the particle pi, and constrains Name to be computed before \
        model evaluation and to propogate derivatives following model   \
        evaluation.                                                      \
        SetupDoc                                                        \
    */                                                                  \
    IMP_DECORATOR_SETUP_1(Name, Refiner *, refiner);                  \
    static bool get_is_setup(kernel::Model *m, ParticleIndex pi) {    \
      return m->get_has_attribute(get_constraint_key(), pi);          \
    }                                                                 \
    IMP_NO_DOXYGEN(typedef boost::false_type DecoratorHasTraits);     \
                                                                      \
   private:                                                           \
    /* hide set methods*/                                             \
    void set_coordinates() {};                                        \
    void set_coordinates_are_optimized() const {}                     \
    void set_coordinate() const {}                                    \
    void set_radius() const {}                                        \
  };                                                                  \
  IMP_DECORATORS(Name, Name##s, Parent##s)


/** See IMP_SUMMARIZE_DECORATOR_DECL()
    \param[in] Name The name for the decorator
    \param[in] Parent the parent decorator type
    \param[in] Members the way to pass a set of particles in
    \param[in] create_pre_modifier the statements to create the
               SingletonModifier which computes the summary info,
               using refiner 'ref'
    \param[in] create_post_modifier a SingletonDerivativeModifier for
               the derivatives of the summary back to its members,
               using refiner 'ref'
*/
#define IMP_SUMMARIZE_DECORATOR_DEF(Name, Parent, Members,                \
                                  create_pre_modifier,                  \
                                  create_post_modifier)                 \
  void Name::do_setup_particle(kernel::Model *m, ParticleIndex pi,      \
                               const ParticleIndexes &pis) {            \
    Refiner *ref = new FixedRefiner(IMP::kernel::get_particles(m, pis)); \
    SingletonModifier* pre_mod = create_pre_modifier;                   \
    SingletonDerivativeModifier* post_mod = create_post_modifier;       \
    if (!Parent::get_is_setup(m, pi)) Parent::setup_particle(m, pi);    \
    set_constraint(pre_mod, post_mod, m, pi);                           \
  }                                                                     \
                                                                        \
  void Name::do_setup_particle(kernel::Model *m, ParticleIndex pi,      \
                               Refiner *ref) {                          \
    SingletonModifier* pre_mod = create_pre_modifier;                   \
    SingletonDerivativeModifier* post_mod = create_post_modifier;       \
    if (!Parent::get_is_setup(m, pi)) Parent::setup_particle(m, pi);    \
    set_constraint(pre_mod, post_mod, m, pi);                           \
  }                                                                     \
                                                                        \
    void Name::show(std::ostream &out) const {                          \
      out << #Name << " at " << static_cast<Parent>(*this);             \
    }                                                                   \
      IMP_CONSTRAINT_DECORATOR_DEF(Name)


/**
   Provided for backward compatability - use
   IMP_SUMMARIZE_DECORATOR_DECL instead.
 */
#define IMP_SUMMARY_DECORATOR_DECL(Name, Parent, Members)    \
   IMPKERNEL_DEPRECATED_MACRO (2.2,                                     \
                 "Use IMP_SUMMARY_DECORATOR_DECL instead"); \
   IMP_SUMMARIZE_DECORATOR_DECL(Name, Parent, Member, )

// for backward compatability
/** See IMP_SUMMARY_DECORATOR_DECL()
    \param[in] Name The name for the decorator
    \param[in] Parent the parent decorator type
    \param[in] Members the way to pass a set of particles in
    \param[in] create_pre_modifier the statements to create the modifier
    which computes the summary info. It should be called 'mod',
    and use the refiner 'ref'
*/
#define IMP_SUMMARY_DECORATOR_DEF(Name, Parent, Members, create_pre_modifier) \
  IMPKERNEL_DEPRECATED_MACRO (2.2,                                      \
         "Explicitly state the constraint used to copy" \
         "derivatives back to the members.");  \
  void Name::do_setup_particle(kernel::Model *m, ParticleIndex pi,      \
                                 const ParticleIndexes &pis) {          \
    Refiner *ref =                                                      \
      new FixedRefiner(IMP::kernel::get_particles(m, pis));             \
    create_pre_modifier;                                                \
    SingletonModifier* post_mod = new DerivativesToRefined(ref);        \
    if (!Parent::get_is_setup(m, pi)) Parent::setup_particle(m, pi);    \
    set_constraint(mod, post_mod, m, pi);                               \
  }                                                                     \
                                                                        \
  void Name::do_setup_particle(kernel::Model *m, ParticleIndex pi,      \
                                 Refiner *ref)                          \
  {                                                                     \
    create_pre_modifier;                                                \
    SingletonModifier* post_mod = new DerivativesToRefined(ref);        \
    if (!Parent::get_is_setup(m, pi)) Parent::setup_particle(m, pi);    \
    set_constraint(mod, post_mod, m , pi);                              \
  }                                                                     \
  void Name::show(std::ostream &out) const {                            \
    out << #Name << " at " << static_cast<Parent>(*this);               \
  }                                                                     \
  IMP_CONSTRAINT_DECORATOR_DEF(Name)




#endif /* IMPKERNEL_DECORATOR_MACROS_H */
