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

namespace IMP
{

typedef std::map<std::string, StringIndex> StringIndexMap;
typedef std::map<std::string, IntIndex> IntIndexMap;
typedef std::map<std::string, FloatIndex> FloatIndexMap;

class Model;

//! Class to handle individual model particles.
/** This class contains particle methods and indexes to particle attributes.
    Particles can be deactivated so that they no longer play a role in model
    optimization. Removing particles and their attributes would cause
    problems in the way attribute values are indexed and should not be done.
 */
class IMPDLLEXPORT Particle : public boost::noncopyable
{
  friend class FloatAttributeIterator;
  friend class IntAttributeIterator;
  friend class StringAttributeIterator;
  friend class Model;
public:

  Particle();
  ~Particle();
 
  //! Get pointer to model particle data.
  /** \return all particle data in the model.
   */
  ModelData* get_model_data(void) const;

  //! Add a Float attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
      \param[in] is_optimized Whether the attribute's value should be
                              optimizable.
   */
  void add_float(const std::string name, const Float value = 0.0,
                 const bool is_optimized = false);

  //! Does particle have a Float attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Float attribute exists in this particle.
   */
  bool has_float(const std::string name) const;

  //! Get the specified Float attribute for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return index to the attribute.
   */
  FloatIndex get_float_index(const std::string name) const;

  //! Add an Int attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
   */
  void add_int(const std::string name, const Int value);

  //! Does particle have an Int attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Int attribute exists in this particle.
   */
  bool has_int(const std::string name) const;

  //! Get the specified Int attribute for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return index to the attribute.
   */
  IntIndex get_int_index(const std::string name) const;

  //! Add a String attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
   */
  void add_string(const std::string name, const String value);

  //! Does particle have a String attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Int attribute exists in this particle.
   */
  bool has_string(const std::string name) const;

  //! Get the specified String attribute for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return index to the attribute.
   */
  StringIndex get_string_index(const std::string name) const;

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
   bool get_is_active(void) const {return is_active_;}

  //! Show the particle
  /** \param[in] out Stream to write particle description to.
   */
  void show(std::ostream& out = std::cout) const;

protected:

  //! Set pointer to model particle data.
  /** This is called by the Model after the particle is added. 
      \param[in] md Pointer to a ModelData object.
   */
  void set_model_data(ModelData *md);

  //! all of the particle data
  ModelData* model_data_;

  //! true if particle is active
  bool is_active_;

  //! float attributes associated with the particle
  std::map<std::string, FloatIndex> float_indexes_;
  //! int attributes associated with the particle
  std::map<std::string, IntIndex> int_indexes_;
  //! string attributes associated with the particle
  std::map<std::string, StringIndex> string_indexes_;
};



inline std::ostream &operator<<(std::ostream &out, const Particle &s) {
  s.show(out);
  return out;
}



//! Iterator for getting all Float attributes from a particle
/** The basic form for use is:

    iter = new FloatAttributeIterator;
    iter->reset(particle_that_i_am_interested_in);
    while (iter->next()) {
      // get next key with iter->get_key()
      // get next value with iter->get_value()
    }
 */
class IMPDLLEXPORT FloatAttributeIterator
{
public:
  FloatAttributeIterator() {}
  ~FloatAttributeIterator() {}

  //! Reset the iterator.
  /** After the next call to next(), get() will return the first Float
      attribute in the Particle.
      \param[in] particle  The Particle that is being referenced.
   */
  void reset(Particle* particle) {
    particle_ = particle;
    reset_ = true;
  }

  //! Move the iterator to the next Float attribute.
  /** \return true if another Float attribute is available.
   */
  bool next(void) {
    if (reset_) {
      cur_ = particle_->float_indexes_.begin();
      reset_ = false;
    } else if (cur_ != particle_->float_indexes_.end())
      ++cur_;

    if (cur_ == particle_->float_indexes_.end())
      return false;

    return true;
  }

  //! Get the key of the Float attribute currently pointed at by the iterator.
  /** \return key of current Float attribute.
   */
  std::string get_key(void) const {
    return cur_->first;
  }

  //! Get the value of the Float attribute currently pointed at by the iterator.
  /** \return value of current Float attribute.
   */
  Float get_value(void) const {
    return particle_->model_data_->get_float(cur_->second);
  }

protected:
  //! map iterator
  FloatIndexMap::iterator cur_;
  Particle* particle_;
  //! flag indicating iterator was reset
  bool reset_;
};


//! Iterator for getting all Int attributes from a particle
class IMPDLLEXPORT IntAttributeIterator
{
public:
  IntAttributeIterator() {}
  ~IntAttributeIterator() {}

  //! Reset the iterator.
  /** After the next call to next(), get() will return the first Int
      attribute in the Particle.
      \param[in] particle  The Particle that is being referenced.
   */
  void reset(Particle* particle) {
    particle_ = particle;
    reset_ = true;
  }

  //! Move the iterator to the next Int attribute.
  /** \return true if another Int attribute is available.
   */
  bool next(void) {
    if (reset_) {
      cur_ = particle_->int_indexes_.begin();
      reset_ = false;
    } else if (cur_ != particle_->int_indexes_.end())
      ++cur_;

    if (cur_ == particle_->int_indexes_.end())
      return false;

    return true;
  }

  //! Get the key of the Int attribute currently pointed at by the iterator.
  /** \return key of current Int attribute.
   */
  std::string get_key(void) const {
    return cur_->first;
  }

  //! Get the value of the Int attribute currently pointed at by the iterator.
  /** \return value of current Int attribute.
   */
  Int get_value(void) const {
    return particle_->model_data_->get_int(cur_->second);
  }

protected:
  IntIndexMap::iterator cur_;
  Particle* particle_;
  bool reset_;
};


//! Iterator for getting all String attributes from a particle
class IMPDLLEXPORT StringAttributeIterator
{
public:
  StringAttributeIterator() {}
  ~StringAttributeIterator() {}


  //! Reset the iterator.
  /** After the next call to next(), get() will return the first String
      attribute in the Particle.
      \param[in] particle  The Particle that is being referenced.
   */
  void reset(Particle* particle) {
    particle_ = particle;
    reset_ = true;
  }

  //! Move the iterator to the next String attribute.
  /** \return true if another String attribute is available.
   */
  bool next(void) {
    if (reset_) {
      cur_ = particle_->string_indexes_.begin();
      reset_ = false;
    } else if (cur_ != particle_->string_indexes_.end())
      ++cur_;

    if (cur_ == particle_->string_indexes_.end())
      return false;

    return true;
  }

  //! Get the key of the String attribute currently pointed at by the iterator.
  /** \return key of current String attribute.
   */
  std::string get_key(void) const {
    return cur_->first;
  }

  //! Get the value of the attribute currently pointed at by the iterator.
  /** \return value of current String attribute.
   */
  String get_value(void) const {
    return particle_->model_data_->get_string(cur_->second);
  }

protected:
  StringIndexMap::iterator cur_;
  Particle* particle_;
  bool reset_;
};

} // namespace IMP

#endif  /* __IMP_PARTICLE_H */
