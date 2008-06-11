/**
 *  \file WormLikeChain.h    \brief Worm-like-chain score for polymer chains.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_WORM_LIKE_CHAIN_H
#define __IMP_WORM_LIKE_CHAIN_H

#include "../UnaryFunction.h"
#include "../internal/constants.h"
#include "../internal/units.h"

namespace IMP
{

//! Worm-like-chain energy for polymer chains
/** This function implements one polymer force/extension curve. It
    is a physical energy and all the inputs are in Angstroms
    and the outputs in kcal/mol (/angstrom).

    \note The actual worm like chain force blows up as the extension approaches
    the maximum. Since that makes optimization problematic, we 
    just linearly extend the force after 99% of maximum.
    \ingroup unaryf
 */
class WormLikeChain : public UnaryFunction
{
public:
  //! Define the energy term
  /** \param[in] l_max maximum length of the chain in angstroms
      \param[in] lp persistence length in angstroms
   */
  WormLikeChain(Float l_max, Float lp) : lmax_(l_max), lp_(lp) {}

  virtual ~WormLikeChain() {}

  //! Calculate the WormLikeChain energy given the length
  /** \param[in] l Current length in Angstroms
      \return Energy in kcal/mol
   */
  virtual Float evaluate(Float lf) {
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
  /** \param[in] l Current length in angstroms
      \param[out] deriv force in kcal/angstrom mol
      \return Score
   */
  virtual Float evaluate_deriv(Float fl, Float& deriv) {
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

    deriv = (du*unit::ATOMS_PER_MOL).get_value();
    //std::cout << "Which converts to " << d << std::endl;
    return evaluate(fl);
  }

  void show(std::ostream &out=std::cout) const {
    out << "WormLikeChain " << lmax_ << " " << lp_ << std::endl;
  }

protected:
  //! \note named to avoid clash with 'deriv' argument
  unit::Piconewton cderiv(unit::Angstrom l) const {
    unit::Piconewton pn= internal::KB*internal::T
      /lp_*(.25/ square(1.0-(l/lmax_).get_normalized_value())
            -.25+(l/lmax_).to_scalar());
     return pn;
  }

  unit::Picojoule eval(unit::Angstrom m) const {
    unit::Picojoule J
      =  internal::KB*internal::T/lp_*(.25*square(lmax_)
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

} // namespace IMP

#endif  /* __IMP_WORM_LIKE_CHAIN_H */
