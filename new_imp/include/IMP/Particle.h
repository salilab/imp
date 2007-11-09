/**
 *  \file Particle.h     \brief Classes to handle individual model particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PARTICLE_H
#define __IMP_PARTICLE_H

#include <map>

#include "IMP_config.h"
#include "Base_Types.h"
#include "ModelData.h"
#include "restraints/Restraint.h"
#include "boost/noncopyable.h"
#include "utility.h"
#include "AttributeKey.h"

namespace IMP
{

//! The key used to identify Float attributes in a Particle
typedef AttributeKey<Float> FloatKey;
//! The key used to identify Int attributes in a Particle
typedef AttributeKey<Int> IntKey;
//! The key used to identify String attributes in a Particle
typedef AttributeKey<String> StringKey;


class Model;

//! Class to handle individual model particles.
/** This class contains particle methods and indexes to particle attributes.
    Particles can be deactivated so that they no longer play a role in model
    optimization. Removing particles and their attributes would cause
    problems in the way attribute values are indexed and should not be done.
 */
class IMPDLLEXPORT Particle : public boost::noncopyable
{
  friend class Model;
public:



  Particle();
  ~Particle();

  //! Get pointer to model particle data.
  /** \return all particle data in the model.
   */
  Model* get_model(void) const;

  //! Add a Float attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
      \param[in] is_optimized Whether the attribute's value should be
                              optimizable.
   */
  void add_attribute(FloatKey name, const Float value = 0.0,
                     const bool is_optimized = false);

  //! Does particle have a Float attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Float attribute exists in this particle.
   */
  bool has_attribute(FloatKey name) const;

  //! Get the specified Float attribute for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return index to the attribute.
   */
  FloatIndex get_attribute(FloatKey name) const;

  //! Add an Int attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
   */
  void add_attribute(IntKey name, const Int value=0);

  //! Does particle have an Int attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Int attribute exists in this particle.
   */
  bool has_attribute(IntKey name) const;

  //! Get the specified Int attribute for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return index to the attribute.
   */
  IntIndex get_attribute(IntKey name) const;

  //! Add a String attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
   */
  void add_attribute(StringKey name, const String value=String());

  //! Does particle have a String attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Int attribute exists in this particle.
   */
  bool has_attribute(StringKey name) const;

  //! Get the specified String attribute for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return index to the attribute.
   */
  StringIndex get_attribute(StringKey  name) const;

  //! Set whether the particle is active.
  /** Restraints referencing the particle are only evaluated for 'active'
      particles.
     \param[in] is_active If true, the particle is active.
   */
  void set_is_active(const bool is_active);

  //! Get whether the particle is active.
  /** Restraints referencing the particle are only evaluated for 'active'
      particles.
      \return true it the particle is active.
   */
  bool get_is_active(void) const {
    return is_active_;
  }

  //! Show the particle
  /** \param[in] out Stream to write particle description to.
   */
  std::ostream& show(std::ostream& out = std::cout) const;

protected:

  //! Set pointer to model particle data.
  /** This is called by the Model after the particle is added.
      \param[in] md Pointer to a ModelData object.
   */
  void set_model(Model *md);

  //! all of the particle data
  Model* model_;

  //! true if particle is active
  bool is_active_;

  //! float attributes associated with the particle
  std::map<FloatKey, FloatIndex> float_indexes_;
  //! int attributes associated with the particle
  std::map<IntKey, IntIndex> int_indexes_;
  //! string attributes associated with the particle
  std::map<StringKey, StringIndex> string_indexes_;
};


IMP_OUTPUT_OPERATOR(Particle)



} // namespace IMP

#endif  /* __IMP_PARTICLE_H */
