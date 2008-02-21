/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Model.h"


namespace IMP
{

//! Constructor
Particle::Particle()
{
  is_active_ = true;
}

//! Constructor
Particle::~Particle()
{
}


//! Set pointer to model particle data.
/** This is called by the Model after the particle is added.
    \param[in] md Pointer to a ModelData object.
 */
void Particle::set_model(Model *md, ParticleIndex pi)
{
  model_ = md;
  pi_ = pi;
}


//! Set whether the particle is active.
/** Restraints referencing the particle are only evaluated for 'active'
    particles.
    \param[in] is_active If true, the particle is active.
 */
void Particle::set_is_active(const bool is_active)
{
  is_active_ = is_active;
}


//! Add a Float attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute.
    \param[in] is_optimized Whether the attribute's value should be optimizable.
 */
void Particle::add_attribute(FloatKey name, const Float value,
                             bool is_optimized)
{
  IMP_assert(model_ ,
             "Particle must be added to Model before an attributes are added");
  float_indexes_.insert(name, model_->get_model_data()->add_float(value));

  model_->get_model_data()->set_is_optimized(float_indexes_.get_value(name),
                                             is_optimized);
}


//! Add an Int attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute.
 */
void Particle::add_attribute(IntKey name, const Int value)
{
  IMP_assert(model_,
             "Particle must be added to Model before an attributes are added");
  int_indexes_.insert(name, model_->get_model_data()->add_int(value));
}


//! Add a String attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute.
 */
void Particle::add_attribute(StringKey name, const String value)
{
  IMP_assert(model_,
             "Particle must be added to Model before an attributes are added");
  string_indexes_.insert(name, model_->get_model_data()->add_string(value));
}



//! Show the particle
/** \param[in] out Stream to write particle description to.
 */
void Particle::show(std::ostream& out) const
{
  char* inset = "  ";
  out << std::endl;
  out << "--" << get_index() << "--" << std::endl;
  if (is_active_) {
    out << inset << inset << "active";
  } else {
    out << inset << inset << "dead";
  }
  out << std::endl;

  if (get_model() != NULL) {
    out << inset << inset << "float attributes:" << std::endl;
    float_indexes_.show(out, "    ", get_model()->get_model_data());

    out << inset << inset << "int attributes:" << std::endl;
    int_indexes_.show(out, "    ", get_model()->get_model_data());

    out << inset << inset << "string attributes:" << std::endl;
    string_indexes_.show(out, "    ", get_model()->get_model_data());
  }
}


}  // namespace IMP
