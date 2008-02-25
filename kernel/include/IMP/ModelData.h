/**
 *  \file ModelData.h    \brief Storage for all model particle data.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MODEL_DATA_H
#define __IMP_MODEL_DATA_H

#include "IMP_config.h"
#include "base_types.h"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <vector>

namespace IMP
{

class Model;
class Particle;
class DerivativeAccumulator;


//! The interface that optimizers use to access Particle attributes.
/** All data for particles is stored in this structure. Float values
    are all differentiable. Each Float value has an associated flag
    saying whether it is optimized or not. Non-optimized values should
    not be changed by the optimizer.

    \note ModelData should only be used in writing Optimizers.
 */
class IMPDLLEXPORT ModelData
{
  friend class Model;
  friend class Particle;
  friend class OptFloatIndexIterator;
  friend class DerivativeAccumulator;
  friend class std::auto_ptr<ModelData>;

  struct FloatData
  {
    Float value_;
    Float deriv_;
    int stats_index_;
    bool is_optimized_;
  };

  struct FloatIsOptimized
  {
    const ModelData *m_;
  public:
    FloatIsOptimized(const ModelData *m): m_(m){}
    bool operator()(FloatIndex f) const {
      return m_->get_is_optimized(f);
    }
  };

  typedef boost::counting_iterator<FloatIndex, 
    std::forward_iterator_tag, unsigned int> FloatIndexIterator;

public:


  //! Add particle float attribute (assumed differentiable) to the model.
  /** The returned index can be used for obtaining and setting the attribute
      value.
      \param[in] value Initial value of the attribute.
      \return index of a new float attribute.
   */
  FloatIndex add_float(const Float value);

  //! Set particle attribute value.
  /** \param[in] idx Index of the attribute.
      \param[in] value Value the attribute should be given.
   */
  void set_value(const FloatIndex idx, const Float value);

  //! Get particle float attribute (inline).
  /** \param[in] idx Index of the particle float attribute.
      \return value of particle float attribute.
   */
  Float get_value(const FloatIndex idx) const {
    IMP_assert(idx.get_index() < float_data_.size(),
               "Out of range index requested");
    return float_data_[idx.get_index()].value_;
  }

  //! Get derivative of the given particle float attribute.
  /** \param[in] idx Index of the particle float attribute.
   */
  Float get_deriv(const FloatIndex idx) const;

  //! Indicate if the particle float attribute is to be optimized.
  /** \param[in] idx Index of the particle float attribute.
      \return True if particle float attribute is to be optimized.
   */
  bool get_is_optimized(const FloatIndex idx) const;

  //! Set whether the particle float attribute is to be optimized.
  /** \param[in] idx Index of the particle float attribute.
      \param[in] is_optimized True if particle float attribute is to be
                              optimized.
   */
  void set_is_optimized(const FloatIndex idx, bool is_optimized);

  //! Set all derivatives to zero.
  void zero_derivatives();

  //! Add particle int attribute to the model.
  /** The returned index can be used for obtaining and setting the
      attribute value.
      \param[in] value Initial value of the attribute.
      \return index of a new int attribute.
   */
  IntIndex add_int(const Int value);

  //! Set particle attribute value.
  /** \param[in] idx Index of the attribute.
      \param[in] value Value the attribute should be given.
   */
  void set_value(const IntIndex idx, const Int value);

  //! Get particle int attribute (inline).
  /** \param[in] idx Index of the particle int attribute.
      \return value of particle float attribute.
   */
  Int get_value(const IntIndex idx) const {
    IMP_assert(idx.get_index() < int_data_.size(),
               "Out of range int requested");
    return int_data_[idx.get_index()];
  }

  //! Add particle string attribute to the model.
  /** The returned index can be used for obtaining and setting the attribute
      value.
      \param[in] value Initial value of the attribute.
      \return index of a new string attribute.
   */
  StringIndex add_string(const String value);

  //! Set particle attribute value.
  /** \param[in] idx Index of the attribute.
      \param[in] value Value the attribute should be given.
   */
  void set_value(const StringIndex idx, const String value);

  //! Get particle string attribute (inline).
  /** \param[in] idx Index of the particle string attribute.
      \return value of particle string attribute.
   */
  String get_value(const StringIndex idx) const {
   IMP_assert(idx.get_index() < string_data_.size(),
               "Out of range string requested");
    return string_data_[idx.get_index()];
  }

  typedef boost::filter_iterator<FloatIsOptimized,
    FloatIndexIterator > OptimizedFloatIndexIterator;
  OptimizedFloatIndexIterator optimized_float_indexes_begin() const {
    return OptimizedFloatIndexIterator(FloatIsOptimized(this),
                                       FloatIndexIterator(0),
                                       FloatIndexIterator(float_data_.size()));
  }
  OptimizedFloatIndexIterator optimized_float_indexes_end() const {
    return OptimizedFloatIndexIterator(FloatIsOptimized(this),
                                       FloatIndexIterator(float_data_.size()),
                                       FloatIndexIterator(float_data_.size()));
  }


  void show(std::ostream &out=std::cout) const;
protected:
  ModelData();
  ~ModelData();

  //! Add value to derivative (used by DerivativeAccumulator).
  /** \param[in] idx Index of the particle float attribute.
      \param[in] value Value to add to the float attribute derivative.
   */
  void add_to_deriv(const FloatIndex idx, const Float value);

  //! particle variables and attributes
  /** these are stored outside of particles to allow
      restraints to get access them directly through
      indexes rather than through particle dereferencing.
   */
  std::vector<FloatData> float_data_;

  //! See float_data_.
  std::vector<Int> int_data_;

  //! See float_data_.
  std::vector<String> string_data_;
};

IMP_OUTPUT_OPERATOR(ModelData);

} // namespace IMP

#endif  /* __IMP_MODEL_DATA_H */
