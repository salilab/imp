/**
 *  \file ModelData.cpp  \brief Storage for all model particle data.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/ModelData.h"
#include "IMP/log.h"

namespace IMP
{

//! Constructor
ModelData::ModelData()
{
}


//! Destructor
ModelData::~ModelData()
{
}


//! Add particle float attribute (assumed differentiable) to the model.
/** The returned index can be used for obtaining and setting the attribute
    value.
    \param[in] value Initial value of the attribute.
    \return index of a new float attribute.
 */
FloatIndex ModelData::add_float(const Float value)
{
  int size = float_data_.size();

  float_data_.resize(size + 1);

  float_data_[size].value_ = value;

  return FloatIndex(size);
}


//! Set particle attribute value.
/** \param[in] idx Index of the attribute.
    \param[in] value Value the attribute should be given.
 */
void ModelData::set_value(const FloatIndex idx, const Float value)
{
  IMP_assert(idx.get_index() < float_data_.size(),
             "Out of range float requested");
  float_data_[idx.get_index()].value_ = value;
}



//! Add value to derivative.
/** \param[in] idx Index of the particle float attribute.
    \param[in] value Value to add to the float attribute derivative.
 */
void ModelData::add_to_deriv(const FloatIndex idx, Float value)
{
  IMP_assert(idx.get_index() < float_data_.size(),
             "Out of range float requested");
  float_data_[idx.get_index()].deriv_ += value;
}


//! Get derivative of the given particle float attribute.
/** \param[in] idx Index of the particle float attribute.
 */
Float ModelData::get_deriv(const FloatIndex idx) const
{
  IMP_assert(idx.get_index() < float_data_.size(),
             "Out of range float requested");
  return float_data_[idx.get_index()].deriv_;
}


//! Indicate if the particle float attribute is to be optimized.
/** \return True if particle float attribute is to be optimized.
 */
bool ModelData::get_is_optimized(const FloatIndex idx) const
{
  IMP_assert(idx.get_index() < float_data_.size(),
             "Out of range float requested");
  return float_data_[idx.get_index()].is_optimized_;
}


//! Set whether the particle float attribute is to be optimized.
/** \param[in] is_optimized True if particle float attribute is to be optimized.
 */
void ModelData::set_is_optimized(const FloatIndex idx, bool is_optimized)
{
  IMP_assert(idx.get_index() < float_data_.size(),
             "Out of range float requested");
  float_data_[idx.get_index()].is_optimized_ = is_optimized;
}


//! Set all derivatives to zero.
void ModelData::zero_derivatives()
{
  for (size_t i = 0; i < float_data_.size(); i++) {
    float_data_[i].deriv_ = (Float) 0.0;
  }
}


//! Add particle int attribute to the model.
/** The returned index can be used for obtaining and setting the
    attribute value.

    \param[in] value Initial value of the attribute.
    \return index of a new int attribute.
 */
IntIndex ModelData::add_int(const Int value)
{
  int size = int_data_.size();

  int_data_.resize(size + 1);
  int_data_[size] = value;

  return IntIndex(size);
}

//! Set particle attribute value.
/** \param[in] idx Index of the attribute.
    \param[in] value Value the attribute should be given.
 */
void ModelData::set_value(const IntIndex idx, const Int value)
{
  IMP_assert(idx.get_index() < int_data_.size(),
             "Out of range int requested");
  int_data_[idx.get_index()] = value;
}


//! Add particle string attribute to the model.
/** The returned index can be used for obtaining and setting the attribute
    value.
    \param[in] value Initial value of the attribute.
    \return index of a new string attribute.
 */
StringIndex ModelData::add_string(const String value)
{
  int size = string_data_.size();

  string_data_.resize(size + 1);

  string_data_[size] = value;

  return StringIndex(size);
}

//! Set particle attribute value.
/** \param[in] idx Index of the attribute.
    \param[in] value Value the attribute should be given.
 */
void ModelData::set_value(const StringIndex idx, const String value)
{
  IMP_assert(idx.get_index() < string_data_.size(),
             "Out of range string requested");
  string_data_[idx.get_index()] = value;
}


void ModelData::show(std::ostream &out) const
{
  out << "Float variables: " << std::endl;
  for (unsigned int i=0; i< float_data_.size(); ++i) {
    out << "  " << float_data_[i].value_ << " " << float_data_[i].deriv_ 
        << (float_data_[i].is_optimized_?" optimized" : " not optimized")
        << std::endl;
  }
}

//! Reset the iterator.
/** After the next call to next(), get() will return the first optimizable
    Float variable.
    \param[in] model_data The model data that is being referenced.
 */
void OptFloatIndexIterator::reset(ModelData* model_data)
{
  model_data_ = model_data;
  cur_ = -1;
}


//! Move to the next optimizable Float variable.
/** Check if another optimizable Float variable is available, and if so,
    make sure it is called by the next call to get().
    \return True if another optimizable variable is available.
 */
bool OptFloatIndexIterator::next()
{
  FloatIndex fi;

  do {
    fi = FloatIndex(++cur_);
    if (cur_ >= (int) model_data_->float_data_.size()) {
      cur_--; // keep pointing at the last item
      return false;
    }

  } while (!model_data_->get_is_optimized(fi));

  return true;
}


//! Return the current available optimizable Float variable.
/** Should only be called if next() returned True.
    \return the index of the Float variable.
 */
FloatIndex OptFloatIndexIterator::get() const
{
  FloatIndex fi;

  fi = FloatIndex(cur_);
  return fi;
}

}  // namespace IMP
