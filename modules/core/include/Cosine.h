/**
 *  \file IMP/core/Cosine.h    \brief Cosine function.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_COSINE_H
#define IMPCORE_COSINE_H

#include <IMP/core/core_config.h>
#include <IMP/UnaryFunction.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

IMPCORE_BEGIN_NAMESPACE

//! %Cosine function.
/** This evaluates the function
    |k| - k cos(nf + a)
    where k is a force constant, n the periodicity, a the phase, and f the
    input value. This is most commonly used for dihedral angle restraints,
    e.g. in the CHARMM force field.
 */
class IMPCOREEXPORT Cosine : public UnaryFunction {
 public:
  //! Constructor.
  /** \param[in] force_constant Force constant (score units)
      \param[in] periodicity Periodicity (generally 1-6)
      \param[in] phase Phase (radians)
   */
  Cosine(Float force_constant, int periodicity, Float phase)
      : force_constant_(force_constant),
        periodicity_(periodicity),
        phase_(phase) {}

  Cosine() {}

  virtual DerivativePair evaluate_with_derivative(
                  double feature) const override;

  virtual double evaluate(double feature) const override;

  IMP_OBJECT_METHODS(Cosine);

  void do_show(std::ostream &out) const;

 private:
  Float force_constant_;
  int periodicity_;
  Float phase_;

  friend class boost::serialization::access;

  template<class T>
  friend struct boost::archive::detail::heap_allocation;

  template<class Archive> void serialize(Archive &ar, const unsigned int) {
    ar & boost::serialization::base_object<UnaryFunction>(*this)
       & force_constant_ & periodicity_ & phase_;
  }    
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_COSINE_H */
