/**
 *  \file utility.h    \brief Various important functionality for implementing
 *                            decorators.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_DECORATOR_UTILITY_H
#define __IMP_DECORATOR_UTILITY_H

//! Define the basic things needed by a Decorator.
/** The key things this defines are a default constructor, a static create
    function, a static cast function, a method get_particle(), a method
    get_model() and comparisons.

    It requires that the implementer of the Decorator implement three static
    methods:
    - static bool has_required_attributes(Particle*) which returns true if
      the Particle has the needed attributes

    - bool add_required_attributes(Particle*) which adds those attributes
      to a particle which does not have them.

    - void initialize_static_data() which initializes static data such as
      AttributeKey instances. Ideally, this should internally make sure it
      is only done once.
 */
#define IMP_DECORATOR(Name)                                             \
  private:                                                              \
  Particle *particle_;                                                  \
  bool is_default() const {return particle_==NULL;}                     \
  Name(Particle* p): particle_(p) {                                     \
    IMP_assert(has_required_attributes(p),                              \
               "This is not a hierarchy particle " << *p);              \
  }                                                                     \
public:                                                                 \
 typedef Name This;                                                     \
 Name(): particle_(NULL){}                                              \
 static Name create(Particle *p) {                                      \
   initialize_static_data();                                            \
   add_required_attributes(p);                                          \
   return Name(p);                                                      \
 }                                                                      \
 static Name cast(Particle *p) {                                        \
   initialize_static_data();                                            \
   if (!has_required_attributes(p)) return Name();                      \
   else return Name(p);                                                 \
 }                                                                      \
 IMP_COMPARISONS_1(particle_)                                           \
 Particle *get_particle() const {return particle_;}                     \
 Model *get_model() const {return particle_->get_model();}              \
private:



//! Perform actions dependent on whether a particle has an attribute.
/** A common pattern is to check if a particle has a particular attribute,
    do one thing if it does and another if it does not. This macro implements
   that pattern. It requires that the method get_particle() return the
   particle being used.

   \param[in] AttributeKey The key for the attribute
   \param[in] has_action The action to take if the Particle has the attribute.
                         The attribute value is stored in the variable VALUE.
   \param[in] not_has_action The action to take if the Particle does not have
                             the attribute.
 */
#define IMP_DECORATOR_GET(AttributeKey, Type, has_action, not_has_action) \
  if (get_particle()->has_attribute(AttributeKey)) {                    \
    Type VALUE =  get_particle()->get_value(AttributeKey);              \
      has_action;                                                       \
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


#endif  /* __IMP_DECORATOR_UTILITY_H */
