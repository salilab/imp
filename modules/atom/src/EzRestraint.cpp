/**
 *  \file EzRestraint.cpp \brief
 *  \brief Ez potential. A statistical scoring function for atom proteins
 *
 *  Copyright 2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/EzRestraint.h>
#include <numeric>
#include <IMP/core/XYZ.h>
#include <IMP/Particle.h>
#include <IMP/atom/internal/Sigmoid.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/internal/Gaussian.h>
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>
#include <string>
#include <iostream>
#include <math.h>

IMPATOM_BEGIN_NAMESPACE

EzRestraint::EzRestraint(Particles ps): Restraint("Ez Potential")
{
 ps_ = ps;
 for(unsigned i=0; i<ps_.size(); ++i){
  // get residue type
  std::string restype=
  atom::Residue(atom::Atom(ps[i]).get_parent()).get_residue_type().get_string();
  // get parameters from residue type
  Floats param=get_parameters(restype);
  // create Sigmoid of Gaussian
  if(restype!="TYR" && restype!="TRP"){
    IMP_NEW(atom::internal::Sigmoid,  ptr, (param[0],param[1],param[2]));
    ufs_.push_back(ptr);
  }else{
    IMP_NEW(atom::internal::Gaussian, ptr, (param[0],param[1],param[2]));
    ufs_.push_back(ptr);
  }
 }
}

Floats EzRestraint::get_parameters(std::string restype)
{
 Floats param(3);
 if(restype=="ALA"){      param[0]= -0.29 ; param[1]= 10.22 ; param[2]= 4.67 ;}
 else if(restype=="ASP"){ param[0]= 1.19 ;  param[1]= 14.25 ; param[2]= 8.98 ;}
 else if(restype=="GLU"){ param[0]= 1.30 ;  param[1]= 14.66 ; param[2]= 4.16 ;}
 else if(restype=="PHE"){ param[0]= -0.80 ; param[1]= 19.67 ; param[2]= 7.12 ;}
 else if(restype=="GLY"){ param[0]= -0.01 ; param[1]= 13.86 ; param[2]= 6.00 ;}
 else if(restype=="HIS"){ param[0]= 0.75 ;  param[1]= 12.26 ; param[2]= 2.77 ;}
 else if(restype=="ILE"){ param[0]= -0.56 ; param[1]= 14.34 ; param[2]= 10.69 ;}
 else if(restype=="LYS"){ param[0]= 1.66 ;  param[1]= 11.11 ; param[2]= 2.09 ;}
 else if(restype=="LEU"){ param[0]= -0.64 ; param[1]= 17.34 ; param[2]= 8.61 ;}
 else if(restype=="MET"){ param[0]= -0.28 ; param[1]= 18.04 ; param[2]= 7.13 ;}
 else if(restype=="ASN"){ param[0]= 0.89 ;  param[1]= 12.78 ; param[2]= 6.28 ;}
 else if(restype=="PRO"){ param[0]= 0.83 ;  param[1]= 18.09 ; param[2]= 3.53 ;}
 else if(restype=="GLN"){ param[0]= 1.21 ;  param[1]= 10.46 ; param[2]= 2.59 ;}
 else if(restype=="ARG"){ param[0]= 1.55 ;  param[1]= 9.34 ;  param[2]= 4.68 ;}
 else if(restype=="SER"){ param[0]= 0.10 ;  param[1]= 13.86 ; param[2]= 6.00 ;}
 else if(restype=="THR"){ param[0]= 0.01 ;  param[1]= 13.86 ; param[2]= 6.00 ;}
 else if(restype=="VAL"){ param[0]= -0.47 ; param[1]= 11.35 ; param[2]= 4.97 ;}
 else if(restype=="TRP"){ param[0]= -0.85 ; param[1]= 11.65 ; param[2]= 7.20 ;}
 else if(restype=="TYR"){ param[0]= -0.42 ; param[1]= 13.04 ; param[2]= 6.20 ;}
 else{std::cout << "No such residue" << std::endl;}
 return param;
}

double EzRestraint::unprotected_evaluate(DerivativeAccumulator *da) const
{
 // check if derivatives are requested
 IMP_USAGE_CHECK(!da, "Derivatives not available");

 double score = 0.0;
 for(unsigned i=0; i<ps_.size(); ++i){
//   if(da){
//    FloatPair score_der =
//    ufs_[i]->evaluate_with_derivative
//(fabs(core::XYZ(ps_[i]).get_coordinate(2)));
//    score += score_der.first;
//    core::XYZ(ps_[i]).add_to_derivative(2,score_der.second,*da);
//   }else{
    score += ufs_[i]->evaluate(fabs(core::XYZ(ps_[i]).get_coordinate(2)));
//   }
 }
 return score;
}

ModelObjectsTemp EzRestraint::do_get_inputs() const {
  ParticlesTemp ret;
  ret.insert(ret.end(), ps_.begin(), ps_.end());
  return ret;
}


IMPATOM_END_NAMESPACE
