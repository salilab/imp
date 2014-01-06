/**
 *  \file SAXSResult.h   \brief A class that stores one docking model
 * with SAXS related scores
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_SAXS_RESULT_H
#define IMP_SAXS_RESULT_H

#include "../../lib/Result.h"

class SAXSResult : public Result {
 public:
  SAXSResult(int number, float chi, bool filtered, float rg,
             IMP::algebra::Transformation3D transformation)
      : Result(number, chi, filtered, 0.0, transformation),
        chi_(chi),
        rg_(rg) {}

  SAXSResult(int number, float chi, bool filtered, float rg, float c1, float c2,
             IMP::algebra::Transformation3D transformation)
      : Result(number, chi, filtered, 0.0, transformation),
        chi_(chi),
        rg_(rg),
        c1_(c1),
        c2_(c2) {}

  float get_chi() const { return chi_; }

  friend std::ostream& operator<<(std::ostream& s, const SAXSResult& p) {
    s.width(6);
    s << p.number_ << " | ";
    s.precision(3);
    s.width(6);
    s << p.chi_;
    if (p.filtered_)
      s << " |  -   | ";
    else
      s << " |  +   | ";
    s.width(6);
    s << p.z_score_ << " | ";
    s.width(6);
    s << p.rg_ << " | ";
    s.width(6);
    s << p.c1_ << " | ";
    s.width(6);
    s << p.c2_ << " | ";
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
    out << "     # | Chi    |filter| Zscore | Rg     | c1     | c2     | "
           "Transformation" << std::endl;
  }

 protected:
  float chi_;
  float rg_;
  float c1_, c2_;  // foxs fitting parameters
};

#endif /* IMP_SAXS_RESULT_H */
