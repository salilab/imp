/**
 * \file SOAPResult.h
 * \brief store SOAP scores for a docking model
 *
 * \authors Dina Schneidman
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_SOAP_RESULT_H
#define IMP_SOAP_RESULT_H

#include "../../lib/Result.h"

class SOAPResult : public Result {
 public:
  SOAPResult(int number, float score, bool filtered, float sas_score,
             float atom_pairs_score,
             IMP::algebra::Transformation3D transformation =
                 IMP::algebra::get_identity_transformation_3d())
      : Result(number, score, filtered, 0.0, transformation),
        sas_score_(sas_score),
        atom_pairs_score_(atom_pairs_score) {}

  void set_sas_score(float sas_score) { sas_score_ = sas_score; }

  friend std::ostream& operator<<(std::ostream& s, const SOAPResult& p) {
    s.width(6);
    s << p.number_ << " | ";
    s.precision(3);
    s.width(6);
    s << p.score_;
    if (p.filtered_)
      s << " |  -   | ";
    else
      s << " |  +   | ";
    s.width(6);
    s << p.z_score_ << " | ";
    s.width(6);
    s << p.sas_score_ << " | ";
    s.width(6);
    s << p.atom_pairs_score_ << " | ";
    IMP::algebra::FixedXYZ euler_angles =
        IMP::algebra::get_fixed_xyz_from_rotation(
            p.transformation_.get_rotation());
    IMP::algebra::Vector3D translation = p.transformation_.get_translation();
    s.precision(4);
    s << euler_angles.get_x() << " " << euler_angles.get_y() << " "
      << euler_angles.get_z() << " " << translation[0] << " " << translation[1]
      << " " << translation[2];
    return s;
  }

  static void print_header(std::ostream& out) {
    out << "     # | SOAP   |filter| Zscore | SAS     | Atom pairs | "
           "Transformation" << std::endl;
  }

 protected:
  float sas_score_;
  float atom_pairs_score_;
};

#endif /* IMP_SOAP_RESULT_H */
