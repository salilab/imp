/**
 *  \file IMP/macros.h    \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_MACROS_H
#define IMPKERNEL_MACROS_H
#include "kernel_config.h"
#include <IMP/base/base_macros.h>

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
                     << #Name << "\n" << *m->get_particle(id));         \
}                                                                       \
explicit Name(::IMP::Particle *p): Parent(p) {                          \
  IMP_INTERNAL_CHECK(particle_is_instance(p),                           \
                     "Particle " << p->get_name()                       \
                     << " missing required attributes for decorator "   \
                     << #Name << "\n" << *p);                           \
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
 IMP_NO_DOXYGEN(typedef Parent ParentDecorator);                        \
 Name(){}                                                               \
 Name(Model *m, ParticleIndex id, const TraitsType &tr): Parent(m, id), \
                                                         traits_(tr) {  \
   IMP_INTERNAL_CHECK(particle_is_instance(m->get_particle(id), tr),    \
                     "Particle " << m->get_particle(id)->get_name()     \
                     << " missing required attributes for decorator "   \
                     << #Name << "\n" << *m->get_particle(id));         \
}                                                                       \
 Name(const TraitsType &tr):                                            \
   traits_(tr) {}                                                       \
 explicit Name(::IMP::Particle *p,                                      \
      const TraitsType &tr=default_traits):                             \
   Parent(p), traits_(tr) {                                             \
   IMP_INTERNAL_CHECK(particle_is_instance(p, tr),                      \
                      "Particle " << p->get_name()                      \
                      << " missing required attributes "                \
                      << " for decorator "                              \
                      << #Name << "\n" << *p);                          \
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
//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
 */
#define IMP_DECORATORS(Name, PluralName, Parent)
#else
#define IMP_DECORATORS(Name, PluralName, Parent)                \
  typedef IMP::Decorators<Name, Parent> PluralName
#endif

#ifdef IMP_DOXYGEN
//! Define the types for storing sets of decorators
/** The macro defines the types PluralName and PluralNameTemp.
 */
#define IMP_DECORATORS_WITH_TRAITS(Name, PluralName, Parent)

#else
#define IMP_DECORATORS_WITH_TRAITS(Name, PluralName, Parent)  \
  typedef IMP::Decorators<Name, Parent> PluralName

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



//! Define the basic things you need for a Restraint.
/** In addition to the methods declared and defined by IMP_OBJECT
    it declares
    - IMP::Restraint::unprotected_evaluate()
    - IMP::Restraint::get_input_containers()
    - IMP::Restraint::get_input_particles()
*/
#define IMP_RESTRAINT(Name)                                             \
  virtual double unprotected_evaluate(DerivativeAccumulator *accum) const; \
  ContainersTemp get_input_containers() const;                          \
  ParticlesTemp get_input_particles() const;                            \
  IMP_OBJECT(Name)

//! Define the basic things you need for an Optimizer.
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::Optimizer::do_optimize()

    \relatesalso IMP::Optimizer
*/
#define IMP_OPTIMIZER(Name)                             \
  virtual Float do_optimize(unsigned int max_steps);   \
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
    - void set_periodicity(unsigned int)

    If you use this macro, you should also include IMP/internal/utility.h.
*/
#define IMP_PERIODIC_OPTIMIZER_STATE(Name)                              \
  virtual void update() {                                               \
    ++call_number_;                                                     \
    if (call_number_%(skip_+1) ==0) {                                   \
      do_update(update_number_);                                        \
      ++update_number_;                                                 \
    }                                                                   \
  }                                                                     \
  void do_update(unsigned int call_number);                             \
  IMP_NO_DOXYGEN(void set_skip_steps(unsigned int k) {set_period(k+1);}); \
  void set_period(unsigned int p) {                                     \
    IMP_USAGE_CHECK(p>0, "Period must be positive.");                   \
    skip_=p-1; call_number_=0;                                          \
  }                                                                     \
  IMP_OBJECT(Name);                                                     \
  private:                                                              \
  ::IMP::internal::Counter skip_, call_number_, update_number_          \

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
          write(oss.str(), false);                                      \
        } else {                                                        \
          write(file_name_, update_number_!=0);                         \
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
    IMP_NO_DOXYGEN(void set_skip_steps(unsigned int k) {set_period(k+1);}); \
    void set_period(unsigned int p) {                                   \
      skip_steps_=p-1;                                                  \
      call_number_=0;                                                   \
    }                                                                   \
void write(std::string file_name, unsigned int call=0,                  \
           bool append=false) const {                                   \
  IMP_UNUSED(call); IMP_UNUSED(append);                                 \
  save_action                                                           \
    }                                                                   \
  private:                                                              \
    void do_update(unsigned int call_number);                           \
    IMP_OBJECT_INLINE(Name##OptimizerState,                             \
                      out << "Write to " << file_name_ << std::endl;,); \
  };                                                                    \
IMP_OBJECTS(Name##OptimizerState, Name##OptimizerStates);               \
/** Write to a file when a failure occurs.*/                            \
class Name##FailureHandler: public base::FailureHandler {               \
  std::string file_name_;                                               \
  vars                                                                  \
  public:                                                               \
  Name##FailureHandler args :                                           \
  base::FailureHandler(std::string("Writer to ")+file_name),            \
    file_name_(file_name) {                                             \
    constr}                                                             \
  functs                                                                \
  void handle_failure() {                                               \
    const std::string file_name=file_name_;                             \
    bool append=false; unsigned int call=0;                             \
    IMP_UNUSED(append); IMP_UNUSED(call);                               \
    save_action                                                         \
      }                                                                 \
  IMP_OBJECT_INLINE(Name##FailureHandler,                               \
                    out << "Write to " << file_name_ << std::endl;,);   \
};                                                                      \
IMP_OBJECTS(Name##FailureHandler, Name##FailureHandlers);


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


#include "singleton_macros.h"
#include "pair_macros.h"
#include "triplet_macros.h"
#include "quad_macros.h"

#endif  /* IMPKERNEL_MACROS_H */
