/*
 *  Particle.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PARTICLE_H
#define __IMP_PARTICLE_H

#include <map>

#include "IMP_config.h"
#include "Base_Types.h"
#include "Model_Data.h"
#include "Restraint.h"

namespace imp
{

typedef std::map<std::string, String_Index> String_Index_Map;
typedef std::map<std::string, Int_Index> Int_Index_Map;
typedef std::map<std::string, Float_Index> Float_Index_Map;

// Particle methods and indexes to particle attributes. Particles can
// be deactivated so that they no longer play a role in model optimization.
// Removing particles and their attributes would cause problems in the way
// attribute values are indexed and should not be done.
class IMPDLLEXPORT Particle
{
  friend class Float_Attribute_Iterator;
  friend class Int_Attribute_Iterator;
  friend class String_Attribute_Iterator;

public:

 
  Particle(Model_Data* model_data);
  ~Particle();
  Model_Data* model_data(void) const;

  // float attributes
  bool add_float(const std::string name, const Float value = 0.0, const bool optimize = false);
  bool has_float(const std::string name) const;
  Float_Index float_index(const std::string name) const;

  // int attributes
  bool add_int(const std::string name, const Int value);
  bool has_int(const std::string name) const;
  Int_Index int_index(const std::string name) const;

  // string attributes
  bool add_string(const std::string name, const String value);
  bool has_string(const std::string name) const;
  String_Index string_index(const std::string name) const;

  // status
  void set_is_active (bool is_active);
  bool is_active (void) const;

  void show (std::ostream& out = std::cout) const;

protected:
  // all of the particle data
  Model_Data* model_data_;

  bool is_active_; // true if particle is active

  // float attributes associated with the particle
  std::map<std::string, Float_Index> float_indexes_;
  // int attributes associated with the particle
  std::map<std::string, Int_Index> int_indexes_;
  // string attributes associated with the particle
  std::map<std::string, String_Index> string_indexes_;
};


/**
  Iterator for getting all Float attributes from a particle

  The basic form for use is:

   iter = new Float_Attribute_Iterator;
   iter->reset(particle_that_i_am_interested_in);
   while (iter->next())
    {
    // get next key with iter->get_key()
    // get next value with iter->get_value()
    }
 */
class IMPDLLEXPORT Float_Attribute_Iterator
{
public:
  Float_Attribute_Iterator() {}
  ~Float_Attribute_Iterator() {}

  /**
    Reset the iterator to iterate over the given particles Float attributes.

    \param[in] particle Pointer to the particle.
   */
  void reset(Particle* particle) {
    particle_ = particle;
    reset_ = true;
  }

  /**
    Move the iterator to the next Float attribute.

    \return true if another Float attribute is available.
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

  /**
    Get the key of the Float attribute currently pointed at
    by the iterator.

    \return key of current Float attribute.
   */
  std::string get_key(void) {
    return cur_->first;
  }

  /**
    Get the value of the Float attribute currently pointed at
    by the iterator.

    \return value of current Float attribute.
   */
  Float get_value(void) {
    return particle_->model_data_->get_float(cur_->second);
  }

protected:
  Float_Index_Map::iterator cur_; // map iterator
  Particle* particle_;
  bool reset_; // flag indicating iterator was reset
};


/**
  Iterator for getting all int attributes from a particle
 */
class IMPDLLEXPORT Int_Attribute_Iterator
{
public:
  Int_Attribute_Iterator() {}
  ~Int_Attribute_Iterator() {}

  /**
    Reset the iterator to iterate over the given particles int attributes.

    \param[in] particle Pointer to the particle.
   */
  void reset(Particle* particle) {
    particle_ = particle;
    reset_ = true;
  }

  /**
    Move the iterator to the next Int attribute.

    \return true if another Int attribute is available.
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

  /**
    Get the key of the Int attribute currently pointed at
    by the iterator.

    \return key of current Int attribute.
   */
  std::string get_key(void) {
    return cur_->first;
  }

  /**
    Get the value of the Int attribute currently pointed at
    by the iterator.

    \return value of current Int attribute.
   */
  Int get_value(void) {
    return particle_->model_data_->get_int(cur_->second);
  }

protected:
  Int_Index_Map::iterator cur_;
  Particle* particle_;
  bool reset_;
};


/**
  Iterator for getting all String attributes from a particle
 */
class IMPDLLEXPORT String_Attribute_Iterator
{
public:
  String_Attribute_Iterator() {}
  ~String_Attribute_Iterator() {}

  /**
    Reset the iterator to iterate over the given particles String attributes.

    \param[in] particle Pointer to the particle.
   */
  void reset(Particle* particle) {
    particle_ = particle;
    reset_ = true;
  }

  /**
    Move the iterator to the next String attribute.

    \return true if another String attribute is available.
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

  /**
    Get the key of the String attribute currently pointed at
    by the iterator.

    \return key of current String attribute.
   */
  std::string get_key(void) {
    return cur_->first;
  }

  /**
    Get the value of the String attribute currently pointed at
    by the iterator.

    \return value of current String attribute.
   */
  String get_value(void) {
    return particle_->model_data_->get_string(cur_->second);
  }

protected:
  String_Index_Map::iterator cur_;
  Particle* particle_;
  bool reset_;
};

} // namespace imp

#endif  /* __IMP_PARTICLE_H */
