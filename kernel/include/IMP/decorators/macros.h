/**
 *  \file decorators/macros.h    \brief Various important macros
 *                                       for implementing decorators.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_DECORATOR_MACROS_H
#define __IMP_DECORATOR_MACROS_H           


//! Define the basic things needed by a Decorator.
/** The key things this defines are a default constructor, a static create
    function, a static cast function, a method get_particle(), a method
    get_model() and comparisons. 

    \param[in] Name is the name of the decorator, such as NameDecorator
    \param[in] Parent The class name for the parent of this class,
    typically DecoratorBase
    \param[in] check_required is code which returns a bool which checks
    if a Particle *p has the required fields
    \param[in] add_required is code which adds the required fields

    It requires that the implementer of the Decorator implement the static
    method:

    - void initialize_static_data() which initializes static data such as
      AttributeKey instances. Ideally, this should internally make sure it
      is only done once.
 */
#define IMP_DECORATOR(Name, Parent, check_required, add_required)       \
  protected:                                                            \
  static bool decorator_keys_initialized_;                              \
  static void decorator_initialize_static_data();                       \
  static bool has_required_attributes(::IMP::Particle *p) {             \
    if (!Parent::has_required_attributes(p)) return false;              \
    check_required;                                                     \
  }                                                                     \
  static void add_required_attributes(::IMP::Particle *p) {             \
    if (!Parent::has_required_attributes(p)) {                          \
      Parent::add_required_attributes(p);                               \
    }                                                                   \
    add_required;                                                       \
  }                                                                     \
  friend class DecoratorBase;                                           \
  Name(::IMP::Particle* p): Parent(p) {                                 \
    IMP_assert(has_required_attributes(p),                              \
               "This is not a particle of type "                        \
               << #Name << *p);                                         \
  }                                                                     \
public:                                                                 \
 typedef Name This;                                                     \
 /** The default constructor. This is used as a null value */           \
 Name(): Parent(){}                                                     \
 /** Add the necessary attributes to p and return a decorator. */       \
 static Name create(::IMP::Particle *p) {                               \
   return IMP::DecoratorBase::create<Name>(p);                          \
 }                                                                      \
 /** Check that p has the necessary attributes and return a decorator.  \
     \throws InvalidStateException if some required attributes are      \
     missing                                                            \
 */                                                                     \
 static Name cast(::IMP::Particle *p) {                                 \
   return IMP::DecoratorBase::cast<Name>(p);                            \
 }                                                                      \
 /** Write information about this decorator to out. Each line should    \
     prefixed by prefix*/                                               \
 void show(std::ostream &out=std::cout,                                 \
           std::string prefix=std::string()) const;   


/**
   Put this macro in to the .cpp with code to initialize the keys
   used by the decorator. This will make sure that the keys are 
   initialized before use and initialized exactly once.
   \param[in] Name the name of the decorate
   \param[in] Parent the name of the parent decorator to make sure its keys
   are initalized
   \param[in] work The list of statements to initialize the keys. 
   this should probably look something like {a_key_=IntKey("Foo");...}
 */
#define IMP_DECORATOR_INITIALIZE(Name, Parent, work)\
  bool Name::decorator_keys_initialized_=false;     \
  void Name::decorator_initialize_static_data() {   \
    if (decorator_keys_initialized_) return;        \
    else {                                          \
      Parent::decorator_initialize_static_data();   \
      work;                                         \
      decorator_keys_initialized_=true;             \
    }                                               \
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


//! Define a set of attributes which form an array
/**
   This macro should go in the header and IMP_DECORATOR_ARRAY_CPP into the .cpp
   and IMP_DECORATOR_ARRAY_INIT in the initialize_static_data function

   To use the array, use functions 

   - Type internal_get_name(unsigned int i) 

   - void internal_add_name(Type) 

   - unsigned int internal_get_number_of_name() const 
 */
#define IMP_DECORATOR_ARRAY_DECL(name, TypeName, Type, Default)         \
  protected:                                                            \
  static IntKey number_of_##name##_key_;                                \
  static std::vector<TypeName##Key> name##_keys_;                       \
  static void generate_##name##_keys(unsigned int i);                   \
  static const TypeName##Key get_##name##_key(unsigned int i) {         \
    if (i >= name##_keys_.size()) generate_##name##_keys(i);            \
    return name##_keys_[i];                                             \
  }                                                                     \
  Type internal_get_##name(unsigned int i) const{                       \
    IMP_DECORATOR_GET(get_##name##_key(i), Type,                        \
                      return VALUE,                                     \
                      throw IndexException("Particle missing attribute"); \
                      return Default);                                 \
  }                                                                     \
  int internal_add_##name(Type t);                                      \
  unsigned int internal_get_number_of_##name() const {                  \
    IMP_DECORATOR_GET(number_of_##name##_key_,                          \
                      Int, return VALUE, return 0);                     \
  }                                                                     \

//! See IMP_DECORATOR_ARRAY_DECL
#define IMP_DECORATOR_ARRAY_DEF(DecoratorType, name, TypeName, Type)   \
  IntKey DecoratorType##Decorator::number_of_##name##_key_;             \
  std::vector<TypeName##Key> DecoratorType##Decorator::name##_keys_;    \
  void DecoratorType##Decorator::generate_##name##_keys(unsigned int i) \
  {                                                                     \
    while (!(i < name##_keys_.size())) {                                \
      std::ostringstream oss;                                           \
      oss << #DecoratorType " " #name " " << name##_keys_.size();       \
      name##_keys_.push_back(TypeName##Key(oss.str().c_str()));         \
    }                                                                   \
  }                                                                     \
  int DecoratorType##Decorator::internal_add_##name(Type t) {           \
    int nc= internal_get_number_of_##name();                            \
    get_particle()->add_attribute(get_##name##_key(nc), t);             \
    IMP_DECORATOR_SET(number_of_##name##_key_, nc+1);                   \
    return nc;                                                          \
  }

//! See IMP_DECORATOR_ARRAY_DECL
#define IMP_DECORATOR_ARRAY_INIT(DecoratorType, name, TypeName, Type)   \
  number_of_##name##_key_= IntKey(#DecoratorType " num " #name);


#endif  /* __IMP_DECORATOR_MACROS_H */
