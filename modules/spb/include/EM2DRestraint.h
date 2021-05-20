/**
 *  \file IMP/spb/EM2DRestraint.h
 *  \brief A restraint for using EM2D cross-corr in a Bayesian way
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_EM2D_RESTRAINT_H
#define IMPSPB_EM2D_RESTRAINT_H
#include <IMP/Pointer.h>
#include <IMP/em2d/Image.h>
#include <IMP/em2d/RegistrationResult.h>
#include <IMP/em2d/project.h>
#include <IMP/isd/Scale.h>
#include <IMP/spb/ISDRestraint.h>
#include <IMP/spb/spb_config.h>

IMPSPB_BEGIN_NAMESPACE
/** A restraint for using EM2D cross-corr in a Bayesian way
    \include  EM2DRestraint.cpp
 */

class IMPSPBEXPORT EM2DRestraint : public spb::ISDRestraint {
  Particles ps_;
  IMP::PointerMember<Particle> sigma_;
  int constr_type_;
  IMP::PointerMember<em2d::Image> image_;
  Float rows_;
  Float cols_;
  em2d::ProjectingOptions options_;
  em2d::RegistrationResults reg_res_;
  Floats fmod_grid_;
  Floats grid_;

  void read_image(std::string ifile, Float px_size, Float res);
  double get_truncated_normalization(double mean, double sigma) const;
  double get_normal_pdf(double mean, double sigma, double x) const;
  void init_grid(const Floats& sigma_grid);
  int get_closest(std::vector<double> const& vec, double value) const;

 public:
  //! EM2D cross correlation is used to score models in a Bayesian way.

  EM2DRestraint(Particles ps, Particle* sigma, std::string ifile, Float px_size,
                Float res);

  EM2DRestraint(Particles ps, Floats sigma_grid, Floats fmod_grid,
                std::string ifile, Float px_size, Float res);

  // set particles
  void set_particles(Particles ps);

  // get cross correlation
  double get_cross_correlation() const;

  // get marginalized likelihood
  double get_marginal_element(double fmod) const;

  // get likelihood
  double get_element(double fmod, double sigma) const;

  // get probability
  double get_probability() const;

  /** This macro declares the basic needed methods: evaluate and show
   */
  // IMP_RESTRAINT(EM2DRestraint);
  //
  /** This macro declares the basic needed methods: evaluate and show
 *    */
  // IMP_RESTRAINT( UniformBoundedRestraint);

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator* accum) const
      IMP_OVERRIDE;
  IMP::ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(EM2DRestraint);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_EM2D_RESTRAINT_H */
