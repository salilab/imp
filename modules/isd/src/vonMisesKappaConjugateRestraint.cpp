/**
 *  \file isd/vonMisesKappaConjugateRestraint.cpp
 *  \brief Restrain a scale particle with log(scale)
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/Particle.h>
#include <IMP/isd/Scale.h>
#include <IMP/isd/vonMisesKappaConjugateRestraint.h>
#include <math.h>
#include <IMP/isd/internal/bessel.h>

IMPISD_BEGIN_NAMESPACE

vonMisesKappaConjugateRestraint::vonMisesKappaConjugateRestraint(Particle *p,
                                                      double c, double R0):
    kappa_(p),c_(c),R0_(R0) {
    if (!(0<=R0 && 0<c && R0<=c)) {
        IMP_THROW("Must have 0 < R0 <= c", ModelException);
    }
}

void vonMisesKappaConjugateRestraint::update_bessel(double kappaval) {
  //compute bessel functions
  I0_ = internal::bessel_i0(kappaval);
  I1_ = internal::bessel_i1(kappaval);
  old_kappaval=kappaval;
}

double vonMisesKappaConjugateRestraint::get_kappa() const
{
    Scale kappascale(kappa_);
    double kappaval=kappascale.get_scale();
    if (kappaval <= 0) {
        IMP_THROW("cannot use conjugate prior on negative or zero scale",
                ModelException);
    }
    if ( kappaval != old_kappaval) {
        const_cast<vonMisesKappaConjugateRestraint*>(this)->update_bessel(
                                                              kappaval);
    }
    return kappaval;

}
double vonMisesKappaConjugateRestraint::get_probability() const
{
    Scale kappascale(kappa_);
    double kappaval=get_kappa();
    //std::cout << "kappa " << kappaval << " c " << c_ << " R0 " << R0_
    //    << " I0 " << I0_ << " exp "  << std::exp(kappaval*R0_)
    //    << " pow " << pow(I0_,-c_) << std::endl;
    if (R0_ > 0) {
        return std::exp(kappaval*R0_)*pow(I0_,-c_);
    } else {
        return pow(I0_,-c_);
    }
}

double vonMisesKappaConjugateRestraint::unprotected_evaluate(
                                  DerivativeAccumulator *accum) const
{
  double score;
  Scale kappascale(kappa_);
  double kappaval=get_kappa();
  if (R0_ > 0) {
      score = -kappaval*R0_ + c_*std::log(I0_);
  } else {
      score = c_*std::log(I0_);
  }

  if (accum) {
    /* calculate derivative and add to 1st coordinate of kappascale */
    double deriv = -R0_ + c_*I1_/I0_;
    kappascale.add_to_scale_derivative(deriv,*accum);
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ParticlesTemp vonMisesKappaConjugateRestraint::get_input_particles() const
{
  return ParticlesTemp(1,kappa_);
}

/* The only container used is pc_. */
ContainersTemp vonMisesKappaConjugateRestraint::get_input_containers() const
{
  return ContainersTemp();
}

void vonMisesKappaConjugateRestraint::do_show(std::ostream& out) const
{
  out << "particle= " << kappa_->get_name() << std::endl;
}

IMPISD_END_NAMESPACE
