/*
 *  Model.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "IMP.h"

namespace imp
{

/**
  Constructor
 */

Model_Data::Model_Data ()
{
  check_particles_active_ = false;
}


/**
  Destructor
 */

Model_Data::~Model_Data ()
{
}



/**
  Add particle float attribute to the model. The returned index can be used
  for obtaining and setting the attribute value.

  \param[in] value Initial value of the attribute.

  \return index of a new float attribute.
 */

Float_Index Model_Data::add_float(const Float value)
{
  Float_Index f_index;
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

void Model_Data::set_float(const Float_Index idx, const Float value)
{
  float_data_[idx.index()].value_ = value;
}


/**
  Get particle float attribute.

  \param[in] idx Index of the particle float attribute.
  \return value of particle float attribute.
 */

Float Model_Data::get_float(const Float_Index idx)
{
  return float_data_[idx.index()].value_;
}


/**
  Add value to deriviative.

  \param[in] idx Index of the particle float attribute.
  \param[in] value Value to add to the float attribute derivative.
 */

void Model_Data::add_to_deriv(const Float_Index idx, Float value)
{
  float_data_[idx.index()].deriv_ += value;
}


/**
  Get deriviative of the given particle float attribute.

  \param[in] idx Index of the particle float attribute.
  */

Float Model_Data::get_deriv(const Float_Index idx)
{
  return float_data_[idx.index()].deriv_;
}


/**
  Set whether the particle float attribute is to be optimized.

  \param[in] is_optimized True if particle float attribute is to be optimized.
  */

void Model_Data::set_is_optimized(const Float_Index idx, bool is_optimized)
{
  float_data_[idx.index()].is_optimized_ = is_optimized;
}


/**
  Indicate in the particle float attribute is to be optimized.

  \return True if particle float attribute is to be optimized.
  */

bool Model_Data::is_optimized(const Float_Index idx)
{
  return float_data_[idx.index()].is_optimized_;
}


/**
  Set all derivatives to zero.
*/

void Model_Data::zero_derivatives(void)
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

Int_Index Model_Data::add_int(const Int value)
{
  Int_Index i_index;
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

void Model_Data::set_int(const Int_Index idx, const Int value)
{
  int_data_[idx.index()] = value;
}


/**
  Get particle int attribute.

  \param[in] idx Index of the particle int attribute.
  \return value of particle float attribute.
 */

Int Model_Data::get_int(const Int_Index idx)
{
  LogMsg(VERBOSE, "get_int: " << int_data_.size() << " " << idx.index());
  return int_data_[idx.index()];
}


/**
  Add particle string attribute to the model. The returned index can be used
  for obtaining and setting the attribute value.

  \param[in] value Initial value of the attribute.

  \return index of a new string attribute.
 */

String_Index Model_Data::add_string(const String value)
{
  String_Index s_index;
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

void Model_Data::set_string(const String_Index idx, const String value)
{
  string_data_[idx.index()] = value;
}


/**
  Get particle string attribute.

  \param[in] idx Index of the particle string attribute.
  \return value of particle string attribute.
 */

String Model_Data::get_string(const String_Index idx)
{
  return string_data_[idx.index()];
}


// ####  Opt_Float_Index_Iterator ####
// Iterator returns all optimizable Floats in the Model_Data


/**
  Reset the iterator so that after the next call to next(),
 get() will return the first optimizable Float variable.

 \param[in] model_data The model data that is being referenced.
 */

void Opt_Float_Index_Iterator::reset(Model_Data* model_data)
{
  model_data_ = model_data;
  cur_ = -1;
}


/**
  Check if another optimizable Float variable is available, and if so,
 make sure it is called by the next call to get().

 \return True if another optimizable variable is available.
 */

bool Opt_Float_Index_Iterator::next(void)
{
  Float_Index fi;

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

Float_Index Opt_Float_Index_Iterator::get(void)
{
  Float_Index fi;

  fi.set_index(cur_);
  return fi;
}



}  // namespace imp

