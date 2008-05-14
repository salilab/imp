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
    static const internal::PicoJoule zero=eval(internal::Angstrom(0));
    internal::Angstrom l(lf);
    if (l < internal::Angstrom(0)) l=internal::Angstrom(0);
    internal::PicoJoule ret;
    if (l < cutoff()) {
      ret= (eval(l) - zero);
    } else {
      internal::PicoJoule springterm=(l-cutoff())*cderiv(cutoff());
      ret= (eval(cutoff())+ springterm -zero);
    }
    /*std::cout << "Return is " << ret <<" " << l << " " << lp_ << " " 
      << lmax_ << std::endl;*/
    return internal::KCalPerMol(convert_to_kcal(ret)).get_value();
  }

  //! Calculate the WormLikeChain energy given the length
  /** \param[in] l Current length in angstroms
      \param[out] deriv force in kcal/angstrom mol
      \return Score
   */
  virtual Float evaluate_deriv(Float fl, Float& deriv) {
    internal::Angstrom l(fl);
    if (l < internal::Angstrom(0)) l=internal::Angstrom(0);
    internal::PicoNewton doubled;
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
    deriv = internal::KCalPerAMol(internal::convert_to_kcal(doubled))
      .get_value();
    //std::cout << "Which converts to " << d << std::endl;
    return evaluate(fl);
  }

  void show(std::ostream &out=std::cout) const {
    out << "WormLikeChain " << lmax_ << " " << lp_ << std::endl;
  }

protected:
  //! \note named to avoid clash with 'deriv' argument
  internal::PicoNewton cderiv(internal::Angstrom l) const {
    internal::PicoNewton pn= internal::KB*internal::T
      /lp_*(internal::Scalar(.25)/ square(internal::Scalar(1)-l/lmax_)
            -internal::Scalar(.25)+l/lmax_);
     return pn;
  }

  internal::PicoJoule eval(internal::Angstrom m) const {
    internal::PicoJoule J
      =  internal::KB*internal::T/lp_*(internal::Scalar(.25)*square(lmax_)
                                       /(lmax_-m)
                                       -m*internal::Scalar(.25) 
                                       +internal::Scalar(.5)*square(m)
                                       /lmax_);
    return J;
  }

  internal::Angstrom cutoff() const {
    return internal::Scalar(.99)*lmax_;
  }

  internal::Angstrom lmax_, lp_;
};

} // namespace IMP

#endif  /* __IMP_WORM_LIKE_CHAIN_H */
