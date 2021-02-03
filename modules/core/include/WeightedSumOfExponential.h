/**
 *  \file IMP/core/WeightedSumOfExponential.h
 *  \brief Negative logarithm of weighted sum of negative exponential of unary functions.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_WEIGHTED_SUM_OF_EXPONENTIAL_H
#define IMPCORE_WEIGHTED_SUM_OF_EXPONENTIAL_H

#include <IMP/core/core_config.h>
#include <IMP/UnaryFunction.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

//! Negative logarithm of weighted sum of negative exponential of unary functions.
/** Given unary functions \f$ f_i(x) \f$ and weights \f$ w_i \f$, compute the function
        \f[ F(x) = -d \log\left[ \sum_i{ \left( w_i e^{-f_i(x) / d} \right) } \right] ,\f]
    where \f$ d \f$ is the denominator of the exponential.
    This is used when the functions \f$ f_i(x) \f$ are scores (\f$ -\log(p) \f$), and the
    desired score \f$ F(x) \f$ is the score resulting from the weighted convolution of
    their probability distributions.
    \see WeightedSum
 */
class WeightedSumOfExponential : public UnaryFunction {
 public:
  /** Create with the functions and their respective weights */
  WeightedSumOfExponential(UnaryFunctions funcs,
                           Floats weights,
                           Float denom = 1.0) : funcs_(funcs), weights_(weights), denom_(denom) {
    IMP_USAGE_CHECK(weights.size() == funcs.size(),
                    "Number of functions and weights must match.");
    IMP_USAGE_CHECK(funcs.size() > 1,
                    "More than one function and weight must be provided.");
    IMP_USAGE_CHECK(denom != 0.,
                    "Exponential denominator must be nonzero.");

  }

  virtual DerivativePair evaluate_with_derivative(double feature) const {
    double exp_sum = 0;
    double derv_num = 0;
    double weight_exp;
    DerivativePair fout;
    for (unsigned int i = 0; i < funcs_.size(); ++i) {
      fout = funcs_[i]->evaluate_with_derivative(feature);
      weight_exp = weights_[i] * std::exp(-fout.first / denom_);
      exp_sum += weight_exp;
      derv_num += weight_exp * fout.second;
    }
    return DerivativePair(-std::log(exp_sum) * denom_, derv_num / exp_sum);
  }

  virtual double evaluate(double feature) const {
    double exp_sum = 0;
    for (unsigned int i = 0; i < funcs_.size(); ++i) {
      exp_sum += weights_[i] * std::exp(-funcs_[i]->evaluate(feature) / denom_);
    }
    return -std::log(exp_sum) * denom_;
  }

  //! Get the number of functions
  unsigned int get_function_number() {
    return funcs_.size();
  }

  //! Set the function weights
  void set_weights(Floats weights) {
    IMP_USAGE_CHECK(weights.size() == get_function_number(),
                    "Number of weights and functions must match.");
    weights_ = weights;
  }

  //! Get the function weights
  Floats get_weights() { return weights_; }

  //! Get function weight at index
  double get_weight(unsigned int i) const {
    IMP_USAGE_CHECK(i < weights_.size(), "Invalid weight index");
    return weights_[i];
  }

  //! Get function at index
  UnaryFunction* get_function(unsigned int i) {
    IMP_USAGE_CHECK(i < get_function_number(), "Invalid function index");
    return funcs_[i];
  }

  //! Set the denominator of the exponential
  void set_denominator(double denom) {
    IMP_USAGE_CHECK(denom != 0.,
                    "Exponential denominator must be nonzero.");
    denom_ = denom;
  }

  //! Get the denominator of the exponential
  double get_denominator() { return denom_; }

  IMP_OBJECT_METHODS(WeightedSumOfExponential);

 private:
  UnaryFunctions funcs_;
  Floats weights_;
  Float denom_;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_WEIGHTED_SUM_OF_EXPONENTIAL_H */
