/**
 *  \file WormLikeChain.h    \brief Worm-like-chain score for polymer chains.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_WORM_LIKE_CHAIN_H
#define IMPMISC_WORM_LIKE_CHAIN_H

#include "config.h"

#include <IMP/UnaryFunction.h>
#include <IMP/internal/constants.h>
#include <IMP/internal/units.h>

IMPMISC_BEGIN_NAMESPACE

//! Worm-like-chain energy for polymer chains
/** This function implements one polymer force/extension curve. It
    is a physical energy and all the inputs are in Angstroms
    and the outputs in kcal/mol (/angstrom).

    \note The actual worm like chain force blows up as the extension approaches
    the maximum. Since that makes optimization problematic, we
    just linearly extend the force after 99% of maximum.
 */
class WormLikeChain : public UnaryFunction
{
public:
  //! Define the energy term
  /** \param[in] l_max maximum length of the chain in angstroms
      \param[in] lp persistence length in angstroms
   */
  WormLikeChain(Float l_max, Float lp) : lmax_(l_max), lp_(lp) {
    IMP_check(l_max > lp, "The persistence length should be less "
              << "than the total length for this model",
              ValueException);
  }

  //! Calculate the WormLikeChain energy given the length
  /** \param[in] lf Current length in Angstroms
      \return Energy in kcal/mol
   */
  virtual Float evaluate(Float lf) const {
    static const unit::Picojoule zero=eval(unit::Angstrom(0));
    unit::Angstrom l(lf);
    if (l < unit::Angstrom(0)) l=unit::Angstrom(0);
    unit::Picojoule ret;
    if (l < cutoff()) {
      ret= (eval(l) - zero);
    } else {
      unit::Picojoule springterm=(l-cutoff())*cderiv(cutoff());
      ret= (eval(cutoff())+ springterm -zero);
    }
    std::cout << "Return is " << ret <<" " << l << " " << lp_ << " "
              << lmax_ << std::endl;
    unit::YoctoKilocalorie zc= convert_J_to_Cal(ret);
    return (zc*unit::ATOMS_PER_MOL).get_value();
  }

  //! Calculate the WormLikeChain energy given the length
  /** \param[in] fl Current length in angstroms
   */
  virtual FloatPair evaluate_with_derivative(Float fl) const {
    unit::Angstrom l(fl);
    if (l < unit::Angstrom(0)) l=unit::Angstrom(0);
    unit::Piconewton doubled;
    if (l < cutoff()) {
      doubled= cderiv(l);
    } else {
      doubled= cderiv(cutoff());
      IMP_LOG(VERBOSE, "Overstretched " << cderiv(cutoff()) << " " << doubled
              << " " << l << " " << lmax_ << " " << cutoff()
              << std::endl);
    }
    //std::cout << "Force is " << doubled << " for length " << l << std::endl;
    // convert from picoNewton
    unit::YoctoKilocaloriePerAngstrom du= unit::convert_J_to_Cal(doubled);

    Float deriv = (du*unit::ATOMS_PER_MOL).get_value();
    //std::cout << "Which converts to " << d << std::endl;
    return std::make_pair(evaluate(fl), deriv);
  }
  /** */
  void show(std::ostream &out=std::cout) const {
    out << "WormLikeChain " << lmax_ << " " << lp_ << std::endl;
  }
  IMP_REF_COUNTED_DESTRUCTOR(WormLikeChain);

private:
  unit::Piconewton cderiv(unit::Angstrom l) const {
    unit::Piconewton pn= IMP::internal::KB*IMP::internal::DEFAULT_TEMPERATURE
      /lp_*(.25/ square(1.0-(l/lmax_).get_normalized_value())
            -.25+(l/lmax_).to_scalar());
     return pn;
  }

  unit::Picojoule eval(unit::Angstrom m) const {
    unit::Picojoule J
      =  IMP::internal::KB *
         IMP::internal::DEFAULT_TEMPERATURE/lp_*(.25*square(lmax_)
                                       /(lmax_-m)
                                       -m*.25
                                       +.5*square(m)
                                       /lmax_);
    return J;
  }

  unit::Angstrom cutoff() const {
    return .99*lmax_;
  }

  unit::Angstrom lmax_, lp_;
};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_WORM_LIKE_CHAIN_H */
