/**
 * \file SOAPResult.h
 * \brief store SOAP scores for a docking model
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_SOAP_RESULT_H
#define IMP_SOAP_RESULT_H

#include <IMP/algebra/Transformation3D.h>

#include <iostream>

class SOAPResult {
public:
  SOAPResult(int number, float score, bool filtered, float sas_score,
             float atom_pairs_score,
             IMP::algebra::Transformation3D transformation) :
    number_(number), score_(score), filtered_(filtered), z_score_(0.0),
    sas_score_(sas_score), atom_pairs_score_(atom_pairs_score),
    transformation_(transformation) {}

  float get_score() const { return score_; }
  bool is_filtered() const { return filtered_; }

  void set_z_score(float z_score) { z_score_ = z_score; }

  friend std::ostream& operator<<(std::ostream& s, const SOAPResult& p) {
    s.width(6); s << p.number_ << " | ";
    s.precision(3); s.width(6); s << p.score_;
    if(p.filtered_) s << " |  -   | "; else s << " |  +   | ";
    s.width(6); s << p.z_score_ << " | ";
    s.width(6); s << p.sas_score_ << " | ";
    s.width(6); s << p.atom_pairs_score_ << " | ";
    IMP::algebra::FixedXYZ euler_angles =
    IMP::algebra::get_fixed_xyz_from_rotation(p.transformation_.get_rotation());
    IMP::algebra::Vector3D translation = p.transformation_.get_translation();
    s.precision(4);
    s << euler_angles.get_x() << " " << euler_angles.get_y() << " "
      << euler_angles.get_z() << " "
      << translation[0] << " " << translation[1] << " " << translation[2];
    return s;
  }

  static void print_header(std::ostream& out) {
    out <<
   "     # | SOAP   |filter| Zscore | SAS     | Atom pairs | Transformation"
        << std::endl;
  }

protected:
  int number_;
  float score_;
  bool filtered_;
  float z_score_;
  float sas_score_;
  float atom_pairs_score_;
  IMP::algebra::Transformation3D transformation_;
};

#endif /* IMP_SOAP_RESULT_H */
