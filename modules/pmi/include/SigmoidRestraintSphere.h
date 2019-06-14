/**
 *  \file IMP/pmi/SigmoidRestraintSphere.h
 *  \brief Simple sigmoidal score calculated between sphere surfaces.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPPMI_SIGMOID_RESTRAINT_SPHERE_H
#define IMPPMI_SIGMOID_RESTRAINT_SPHERE_H
#include "pmi_config.h"
#include <IMP/Restraint.h>
#include <IMP/particle_index.h>


IMPPMI_BEGIN_NAMESPACE

//! Simple sigmoidal score calculated between sphere surfaces.
class IMPPMIEXPORT  SigmoidRestraintSphere : public Restraint
{
    IMP::ParticleIndex p1_;
    IMP::ParticleIndex p2_;
    double inflection_;
    double slope_;
    double amplitude_;
    double line_slope_;



public:


  //! Create the restraint.
  IMPPMI_DEPRECATED_OBJECT_DECL(2.11)
  SigmoidRestraintSphere(IMP::Model *m, 
                          IMP::ParticleIndexAdaptor p1,
                          IMP::ParticleIndexAdaptor p2,
                          double inflection, double slope, 
                          double amplitude, double line_slope_=0,
                     std::string name="SigmoidRestraintSphere%1%");

  void set_amplitude(double amplitude){amplitude_=amplitude;}
  void increment_amplitude(double amplitude){amplitude_=amplitude_+amplitude;}  
  double get_amplitude(){return amplitude_;}

  virtual double
  unprotected_evaluate(IMP::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(SigmoidRestraintSphere);


};

IMPPMI_END_NAMESPACE

#endif  /* IMPPMI_SIGMOID_RESTRAINT_SPHERE_H */
