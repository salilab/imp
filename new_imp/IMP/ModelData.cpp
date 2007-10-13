/*
 *  ModelData.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "ModelData.h"
#include "log.h"

namespace IMP
{

/**
  Constructor
 */

ModelData::ModelData ()
{
  check_particles_active_ = false;
}


/**
  Destructor
 */

ModelData::~ModelData ()
{
}



/**
  Add particle float attribute to the model. The returned index can be used
  for obtaining and setting the attribute value.

  \param[in] value Initial value of the attribute.

  \return index of a new float attribute.
 */

FloatIndex ModelData::add_float(const Float value)
{
  FloatIndex f_index;
  int size = float_data_.size();

  float_data_.resize(size+1);

  f_index.set_index(size);
  float_data_[f_index.index()].value_ = value;

  return f_index;
}


/**
  Set particle attribute value.

  \param[in] idx Index of the attribute.
  \param[in] value Value the attribute should be given.
 */

void ModelData::set_float(const FloatIndex idx, const Float value)
{
  float_data_[idx.index()].value_ = value;
}



/**
  Add value to deriviative.

  \param[in] idx Index of the particle float attribute.
  \param[in] value Value to add to the float attribute derivative.
 */

void ModelData::add_to_deriv(const FloatIndex idx, Float value)
{
  float_data_[idx.index()].deriv_ += value;
}


/**
  Get deriviative of the given particle float attribute.

  \param[in] idx Index of the particle float attribute.
  */

Float ModelData::get_deriv(const FloatIndex idx) const
{
  return float_data_[idx.index()].deriv_;
}


/**
  Set whether the particle float attribute is to be optimized.

  \param[in] is_optimized True if particle float attribute is to be optimized.
  */

void ModelData::set_is_optimized(const FloatIndex idx, bool is_optimized)
{
  float_data_[idx.index()].is_optimized_ = is_optimized;
}


/**
  Indicate in the particle float attribute is to be optimized.

  \return True if particle float attribute is to be optimized.
  */

bool ModelData::is_optimized(const FloatIndex idx) const
{
  return float_data_[idx.index()].is_optimized_;
}


/**
  Set all derivatives to zero.
*/

void ModelData::zero_derivatives(void)
{
  for (size_t i = 0; i < float_data_.size(); i++) {
    float_data_[i].deriv_ = (Float) 0.0;
  }
}


/**
  Add particle int attribute to the model. The returned index can be used
  for obtaining and setting the attribute value.

  \param[in] value Initial value of the attribute.

  \return index of a new int attribute.
 */

IntIndex ModelData::add_int(const Int value)
{
  IntIndex i_index;
  int size = int_data_.size();

  int_data_.resize(size+1);

  i_index.set_index(size);
  int_data_[i_index.index()] = value;

  return i_index;
}

/**
  Set particle attribute value.

  \param[in] idx Index of the attribute.
  \param[in] value Value the attribute should be given.
 */

void ModelData::set_int(const IntIndex idx, const Int value)
{
  int_data_[idx.index()] = value;
}


/**
  Add particle string attribute to the model. The returned index can be used
  for obtaining and setting the attribute value.

  \param[in] value Initial value of the attribute.

  \return index of a new string attribute.
 */

StringIndex ModelData::add_string(const String value)
{
  StringIndex s_index;
  int size = string_data_.size();

  string_data_.resize(size+1);

  s_index.set_index(size);
  string_data_[s_index.index()] = value;

  return s_index;
}

/**
  Set particle attribute value.

  \param[in] idx Index of the attribute.
  \param[in] value Value the attribute should be given.
 */

void ModelData::set_string(const StringIndex idx, const String value)
{
  string_data_[idx.index()] = value;
}


// ####  OptFloatIndexIterator ####
// Iterator returns all optimizable Floats in the ModelData


/**
  Reset the iterator so that after the next call to next(),
 get() will return the first optimizable Float variable.

 \param[in] model_data The model data that is being referenced.
 */

void OptFloatIndexIterator::reset(ModelData* model_data)
{
  model_data_ = model_data;
  cur_ = -1;
}


/**
  Check if another optimizable Float variable is available, and if so,
 make sure it is called by the next call to get().

 \return True if another optimizable variable is available.
 */

bool OptFloatIndexIterator::next(void)
{
  FloatIndex fi;

  do {
    fi.set_index(++cur_);
    if (cur_ >= (int) model_data_->float_data_.size()) {
      cur_--; // keep pointing at the last item
      return false;
    }

  } while (!model_data_->is_optimized(fi));

  return true;
}


/**
  Return the next available optimizable Float variable. Should only
  be called if next() returned True.

 \return True if another optimizable variable is available.
 */

FloatIndex OptFloatIndexIterator::get(void) const
{
  FloatIndex fi;

  fi.set_index(cur_);
  return fi;
}



}  // namespace IMP

