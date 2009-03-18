/**
 *  \file IMP/macros.h    \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_MACROS_H
#define IMP_MACROS_H

//! Implement comparison in a class using a compare function
/** The macro requires that This be defined as the type of the current class.
    The compare function should take a const This & and return -1, 0, 1 as
    appropriate.
 */
#define IMP_COMPARISONS                                                 \
  bool operator==(const This &o) const {                                \
    return (compare(o) == 0);                                           \
  }                                                                     \
  bool operator!=(const This &o) const {                                \
    return (compare(o) != 0);                                           \
  }                                                                     \
  bool operator<(const This &o) const {                                 \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (compare(o) <0);                                             \
  }                                                                     \
  bool operator>(const This &o) const {                                 \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (compare(o) > 0);                                            \
  }                                                                     \
  bool operator>=(const This &o) const {                                \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return !(compare(o) < 0);                                           \
  }                                                                     \
  bool operator<=(const This &o) const {                                \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return !(compare(o) > 0);                                           \
  }

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_1(field)                                        \
   bool operator==(const This &o) const {                         \
    return (field== o.field);                                           \
  }                                                                     \
   bool operator!=(const This &o) const {                         \
    return (field!= o.field);                                           \
  }                                                                     \
   bool operator<(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field< o.field);                                            \
  }                                                                     \
   bool operator>(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field> o.field);                                            \
  }                                                                     \
   bool operator>=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field>= o.field);                                           \
  }                                                                     \
   bool operator<=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field<= o.field);                                           \
  }

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_2(f0, f1)                                       \
  bool operator==(const This &o) const {                                \
    return (f0== o.f0 && f1==o.f1);                                     \
  }                                                                     \
  bool operator!=(const This &o) const {                                \
    return (f0!= o.f0 || f1 != o.f1);                                   \
  }                                                                     \
  bool operator<(const This &o) const {                                 \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    else return f1 < o.f1;                                              \
  }                                                                     \
  bool operator>(const This &o) const {                                 \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    else return f1 > o.f1;                                              \
  }                                                                     \
  bool operator>=(const This &o) const {                                \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  bool operator<=(const This &o) const {                                \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator<(o) || operator==(o);                               \
  }

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_3(f0, f1, f2)                                   \
  bool operator==(const This &o) const {                                \
    return (f0== o.f0 && f1==o.f1 && f2 == o.f2);                       \
  }                                                                     \
  bool operator!=(const This &o) const {                                \
    return (f0!= o.f0 || f1 != o.f1 || f2 != o.f2);                     \
  }                                                                     \
  bool operator<(const This &o) const {                                 \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    if (f1< o.f1) return true;                                          \
    else if (f1 > o.f1) return false;                                   \
    else return f2 < o.f2;                                              \
  }                                                                     \
  bool operator>(const This &o) const {                                 \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    if (f1 > o.f1) return true;                                         \
    else if (f1 < o.f1) return false;                                   \
    else return f2 > o.f2;                                              \
  }                                                                     \
  bool operator>=(const This &o) const {                                \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  bool operator<=(const This &o) const {                                \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator<(o) || operator==(o);                               \
  }

//! Implement operator<< on class name, assuming it has one template argument
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR_1(name)                                     \
template <class L>                                                      \
 inline std::ostream& operator<<(std::ostream &out, const name<L> &i)   \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }

//! Implement operator<< on class name, assuming it has two template arguments
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR_2(name)                                     \
  template <class L, class M>                                           \
  inline std::ostream& operator<<(std::ostream &out, const name<L, M> &i) \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }

//! Implement operator<< on class name
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR(name)                                       \
  inline std::ostream &operator<<(std::ostream &out, const name &i)     \
  {                                                                     \
    i.show(out);                                                 \
    return out;                                                  \
  }

//! Define the basic things you need for a Restraint.
/** These are: show, evaluate, get_version_info and a empty destructor
    \param[in] Name The class name
    \param[in] version_info The version info object to return.
    \relates IMP::Restraint
*/
#define IMP_RESTRAINT(Name, version_info)                                \
  virtual Float evaluate(DerivativeAccumulator *accum);                 \
  virtual void show(std::ostream &out=std::cout) const;                 \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }\
  IMP_REF_COUNTED_DESTRUCTOR(Name)                                      \
  public:


//! Define the basic things you need for an optimizer.
/** These are: optimize, get_version_info
    \param[in] version_info The version info object to return.
*/
#define IMP_OPTIMIZER(version_info)                                     \
  /** \short Optimize the model.
      \param[in] max_steps The maximum number of steps to take.
      \return The final score.
   */                                                                   \
  virtual Float optimize(unsigned int max_steps);                       \
  /** \return version and authorship information */                     \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }


//! Define the basics needed for an OptimizerState
/** This macro declares the required functions
    - void update()
    - void show(std::ostream &out) const
    and defines the function
    - get_version_info
    \param[in] version_info The version info object to return.
*/
#define IMP_OPTIMIZER_STATE(version_info)                               \
  virtual void update();                                                \
  virtual void show(std::ostream &out=std::cout) const;                 \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }

//! Define the basics needed for a ScoreState
/** This macro declares the required functions
    - void do_before_evaluate()
    - void show(std::ostream &out) const
    and defines the function
    - get_version_info
    - an empty destructor

    \relates IMP::ScoreState

    \param[in] Name the class name
    \param[in] version_info The version info object to return.
*/
#define IMP_SCORE_STATE(Name, version_info)                              \
protected:                                                              \
 virtual void do_before_evaluate();                                     \
 virtual void do_after_evaluate(DerivativeAccumulator *da);             \
 IMP_REF_COUNTED_DESTRUCTOR(Name)                                      \
public:                                                                 \
 virtual void show(std::ostream &out=std::cout) const;                  \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }

//! Define the basics needed for a particle refiner
/** This macro declares the following functions
    - bool can_refine(Particle*) const;
    - void cleanup(Particle *a, Particles &b,
                   DerivativeAccumulator *da);
    - void show(std::ostream &out) const;
    - Particles refine(Particle *) const;

    \param[in] version_info The version info object to return

 */
#define IMP_PARTICLE_REFINER(version_info)                              \
  public:                                                               \
  virtual bool get_can_refine(Particle*) const;                         \
  virtual void show(std::ostream &out) const;                           \
  virtual void cleanup_refined(Particle *a, Particles &b,               \
                               DerivativeAccumulator *da=0) const;      \
  virtual Particles get_refined(Particle *) const;                      \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }

//! Use the swap_with member function to swap two objects
#define IMP_SWAP(name) \
  inline void swap(name &a, name &b) {          \
    a.swap_with(b);                             \
  }

//! swap two member variables assuming the other object is called o
#define IMP_SWAP_MEMBER(var_name) \
  std::swap(var_name, o.var_name);



//! use a copy_from method to create a copy constructor and operator=
#define IMP_COPY_CONSTRUCTOR(TC) TC(const TC &o){copy_from(o);}  \
  TC& operator=(const TC &o) {copy_from(o); return *this;}



#ifdef _MSC_VER
// VC doesn't understand friends properly
#define IMP_REF_COUNTED_DESTRUCTOR(Classname)                   \
public:                                                         \
 virtual ~Classname(){}
#else

#if defined(SWIG) || defined(IMP_SWIG_WRAPPER)
// SWIG doesn't do friends right either, but we don't care as much
#define IMP_REF_COUNTED_DESTRUCTOR(Classname)                   \
public:                                                      \
 virtual ~Classname(){}
#else
//! Declare a protected destructor and get the friends right
/** The destructor is unprotected for SWIG since if it is protected
    SWIG does not wrap the python proxy distruction and so does not
    dereference the ref counted pointer. Swig also gets confused
    on template friends.
 */
#define IMP_REF_COUNTED_DESTRUCTOR(Classname)                   \
  protected:                                                    \
  template <class T> friend void IMP::internal::disown(T*);     \
  friend class IMP::internal::UnRef<true>;                      \
  virtual ~Classname(){}
#endif // SWIG
#endif // _MSC_VER


//! Define the basic things needed by a Decorator.
/** The key things this defines are
    - a default constructor,
    - a static cast function,
    - a method get_particle(),
    - a method get_model()
    - comparisons.
    - a show method

    \param[in] Name is the name of the decorator, such as NameDecorator
    \param[in] Parent The class name for the parent of this class,
    typically Decorator

    It requires that the implementer of the Decorator implement the static
    methods:

    - bool is_instance_of(Particle *p) which checks if a particle has
    needed attributes.
    - create(Particle *p, other args) which adds the needed attributes
    to a particle
 */
#define IMP_DECORATOR(Name, Parent)                                     \
public:                                                                 \
/** \note Should be private but SWIG accesses it through the comparison
    macros*/                                                            \
 typedef Name This;                                                     \
 /** \short The default constructor. This is used as a null value */    \
 Name(): Parent(){}                                                     \
 /** \short Construct from a Particle which has all needed attributes */\
explicit Name(::IMP::Particle *p): Parent(p) {                          \
   IMP_assert(is_instance_of(p),                                        \
              "Particle missing required attributes for decorator "     \
              << #Name << *p << std::endl);                             \
 }                                                                      \
 /** Check that p has the necessary attributes and return a decorator.
     \throws InvalidStateException if some required attributes are
     missing
 */                                                                     \
 static Name cast(::IMP::Particle *p) {                                 \
   IMP_CHECK_OBJECT(p);                                                 \
   if (!is_instance_of(p)) {                                            \
      throw InvalidStateException("Particle missing required attributes"\
                                  " in cast");                          \
   }                                                                    \
   return Name(p);                                                      \
 }                                                                      \
 /** Write information about this decorator to out. Each line should
     prefixed by prefix*/                                               \
 void show(std::ostream &out=std::cout,                                 \
           std::string prefix=std::string()) const;

//! Define the basic things needed by a Decorator which has a traits object.
/** The key things this defines are
    - a default constructor,
    - a static cast function,
    - a method get_particle(),
    - a method get_model()
    - comparisons.
    - a show method
    - a get_traits_name method

    \param[in] Name is the name of the decorator, such as NameDecorator
    \param[in] Parent The class name for the parent of this class,
    typically Decorator
    \param[in] TraitsType the type of the traits object
    \param[in] traits_name what to name the traits object.
    \param[in] default_traits How to get the default traits value

    It requires that the implementer of the Decorator implement the static
    methods:

    - bool is_instance_of(Particle *p) which checks if a particle has
    needed attributes.
    - create(Particle *p, other args) which adds the needed attributes
    to a particle
 */
#define IMP_DECORATOR_TRAITS(Name, Parent, TraitsType, traits_name,     \
default_traits)                                                         \
  private:                                                              \
  TraitsType traits_name##_;                                             \
public:                                                                 \
/** \note Should be private but SWIG accesses it through the comparison
    macros*/                                                            \
 typedef Name This;                                                     \
 /** \short The default constructor. This is used as a null value */    \
 Name(): Parent(){}                                                     \
 /** \short Construct from a Particle which has all needed attributes */\
Name(::IMP::Particle *p, const TraitsType &tr=default_traits): Parent(p), \
                                                traits_name##_(tr) {     \
  IMP_assert(is_instance_of(p, tr),                                     \
              "Particle missing required attributes for decorator "     \
              << #Name << *p << std::endl);                             \
 }                                                                      \
 /** Check that p has the necessary attributes and return a decorator.
     \throws InvalidStateException if some required attributes are
     missing
 */                                                                     \
static Name cast(::IMP::Particle *p, const TraitsType &tr=default_traits) { \
  IMP_check(is_instance_of(p, tr), "Particle missing required attributes for "\
  << "decorator " << #Name << " " << *p, InvalidStateException);\
  return Name(p, tr);\
}                                                                       \
 /** Write information about this decorator to out. Each line should
     prefixed by prefix*/                                               \
 void show(std::ostream &out=std::cout,                                 \
           std::string prefix=std::string()) const;                     \
/** Get the traits object */                                            \
const TraitsType &get_##traits_name() const {                            \
  return traits_name##_;                                                 \
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
 */
#define IMP_DECORATOR_GET(AttributeKey, Type, has_action, not_has_action) \
  if (get_particle()->has_attribute(AttributeKey)) {                    \
    Type VALUE =  get_particle()->get_value(AttributeKey);              \
    has_action;                                                         \
  } else {                                                              \
    not_has_action;                                                     \
  }


//! Set an attribute, creating it if it does not already exist.
/** Another commont pattern is to have an assumed value if the attribute
    is not there. Then, you sometimes need to set the value whether it
    is there or not.
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
*/
#define IMP_DECORATOR_GET_SET(name, AttributeKey, Type, ReturnType)     \
  /** \return the attribute value  */                                   \
  ReturnType get_##name() const {                                       \
    return static_cast<ReturnType>(get_particle()->get_value(AttributeKey)); \
  }                                                                     \
  /** Set the attribute \param[in] t the value    */                    \
  void set_##name(ReturnType t) {                                             \
    get_particle()->set_value(AttributeKey, t);                         \
  }

//! Define methods for getting and setting an optional simple field.
/**
   See IMP_DECORATOR_GET_SET.

   \param[in] name The lower case name of the attribute
   \param[in] AttributeKey The AttributeKey object controlling
                           the attribute.
   \param[in] Type The type of the attribute (upper case).
   \param[in] ReturnType The type to return from the get.
   \param[in] default_value The value returned if the attribute is missing.
 */
#define IMP_DECORATOR_GET_SET_OPT(name, AttributeKey, Type,             \
                                  ReturnType, default_value)            \
  /** \return the attribute value*/                                     \
  ReturnType get_##name() const {                                       \
    IMP_DECORATOR_GET(AttributeKey, Type,                               \
                      return static_cast<ReturnType>(VALUE),            \
                      return default_value);                            \
  }                                                                     \
  /** \param[in] t the value to set the attribute to*/                  \
  void set_##name(ReturnType t) {                                       \
    IMP_DECORATOR_SET(AttributeKey, t);                                 \
  }

              /* static internal::ArrayOnAttributesHelper<Traits::Key,  \
                 Traits::Value> name##_data_;  */

//! Define a set of attributes which form an array
/**
   This macro should go in the header and IMP_DECORATOR_ARRAY_CPP into the .cpp
   and IMP_DECORATOR_ARRAY_INIT in the initialize_static_data function

   The macro defines a set of functions for using the array:
   - get_name(unsigned int)

   - get_number_of_name()

   - add_name(ExternalType)

   - add_name_at(ExternalType, unsigned int)

   - remove_name(unsigned int)

   in addition it defines the private methods
   - add_required_attributes_for_name(Particle *)

   \param[in] protection Whether it should be public, protected or private
   \param[in] name the name prefix to use, see the above method names
   \param[in] plural the plural form of the name
   \param[in] traits the traits object to use to manipulate things. This should
   inherit from or implement the interface of internal::ArrayOnAttributesHelper
   \param[in] ExternalType The name of the type to wrap the return type with.
   \param[in] helper The internal::ArrayOnAttributesHelper instance to use.
 */
#define IMP_DECORATOR_ARRAY_DECL(protection, name, plural,              \
                                 traits, ExternalType)                  \
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
protection:                                                             \
 /** \brief Get the ith member*/                                        \
 ExternalType get_##name(unsigned int i) const {                        \
   return traits.wrap(traits.get_value(get_particle(), i));             \
 }                                                                      \
 /** \brief Get the total number of them*/                              \
 unsigned int get_number_of_##plural() const {                          \
   return traits.get_size(get_particle());                              \
 }                                                                      \
 /** \brief Add t at the end */                                         \
 unsigned int add_##name(ExternalType t) {                              \
   traits.audit_value(t);                                               \
   unsigned int i= traits.push_back(get_particle(),                     \
                                    traits.get_value(t));               \
   traits.on_add(get_particle(), t, i);                                 \
   return i;                                                            \
 }                                                                      \
 /** Add t at a certain position */                                     \
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
 /** Remove t from the array */                                         \
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
 }


//! add a method to get a key
/** One has to make sure to call the
    decorator_initialize_static_data method first
 */
#define IMP_DECORATOR_GET_KEY(KeyType, key_name, key_string)   \
  static KeyType get_##key_name() {                            \
    static KeyType k(#key_string);                             \
    return k;                                                  \
  }




//! Define the functions needed for a SingletonModifier
/** \see IMP_SINGLETON_MODIFIER_DA
 */
#define IMP_SINGLETON_MODIFIER(version)                          \
  VersionInfo get_version_info() const {return version;}         \
  void show(std::ostream &out= std::cout) const;                 \
  void apply(Particle *a) const;                                 \
  void apply(Particle *a, DerivativeAccumulator*) const{         \
    apply(a);                                                    \
  }



//! Define the functions needed for a PairModifier
/** \see IMP_PAIR_MODIFIER_DA
 */
#define IMP_PAIR_MODIFIER(version)                                      \
  VersionInfo get_version_info() const {return version;}                \
  void show(std::ostream &out= std::cout) const;                        \
  void apply(Particle *a, Particle *b) const;                           \
  void apply(Particle *a, Particle *b, DerivativeAccumulator*) const{     \
    apply(a,b);                                                         \
  }

//! Define the functions needed for a SingletonModifier
/** This version takes a derivative accumulator.
   \see IMP_SINGLETON_MODIFIER
 */
#define IMP_SINGLETON_MODIFIER_DA(version)                            \
  VersionInfo get_version_info() const {return version;}              \
  void show(std::ostream &out= std::cout) const;                      \
  void apply(Particle *a, DerivativeAccumulator *da) const;           \
  void apply(Particle *) const{                                       \
    IMP_failure("This modifier requires a derivative accumulator "    \
               << *this, ErrorException);                             \
 }


//! Define the functions needed for a PairModifier
/** This version takes a derivative accumulator.
    \see IMP_PAIR_MODIFIER
 */
#define IMP_PAIR_MODIFIER_DA(version) \
  VersionInfo get_version_info() const {return version;}                \
 void show(std::ostream &out= std::cout) const;                         \
 void apply(Particle *a, Particle *b, DerivativeAccumulator *da) const; \
 void apply(Particle *, Particle *) const{                              \
   IMP_failure("This modifier requires a derivative accumulator "       \
               << *this, ErrorException);                               \
 }
//! Define the needed functions for a SingletonContainer
/** You need to implement
    - get_contains_particle
    - get_number_of_particles
    - get_particle
    - show
    A private, empty destructor is provided.
    \relates IMP::SingletonContainer
*/
#define IMP_SINGLETON_CONTAINER(Name, version_info)                  \
  bool get_contains_particle(Particle* p) const;                    \
  unsigned int get_number_of_particles() const;                     \
  Particle* get_particle(unsigned int i) const;                     \
  void show(std::ostream &out= std::cout) const;                    \
  IMP::VersionInfo get_version_info() const { return version_info; }\
  IMP_REF_COUNTED_DESTRUCTOR(Name)                                  \
  public:

//! Define the needed functions for a PairContainer
/** \relates IMP::PairContainer
    See IMP_SINGLETON_CONTAINER() for full documentation.
 */
#define IMP_PAIR_CONTAINER(Name, version_info)                           \
  bool get_contains_particle_pair(ParticlePair p) const;                \
  unsigned int get_number_of_particle_pairs() const;                    \
  ParticlePair get_particle_pair(unsigned int i) const;                 \
  void show(std::ostream &out= std::cout) const;                        \
  IMP::VersionInfo get_version_info() const { return version_info; }    \
  IMP_REF_COUNTED_DESTRUCTOR(Name)                                      \
  public:


#endif  /* IMP_MACROS_H */
