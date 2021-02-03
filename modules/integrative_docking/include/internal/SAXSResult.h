/**
 *  \file IMP/integrative_docking/SAXSResult.h
 *  \brief A class that stores one docking model with SAXS related scores
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_SAXS_RESULT_H
#define IMPINTEGRATIVE_DOCKING_SAXS_RESULT_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include "Result.h"

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

class SAXSResult : public Result {
 public:
  SAXSResult(int number, float chi, bool filtered, float rg,
             IMP::algebra::Transformation3D transformation)
      : Result(number, chi, filtered, 0.0, transformation),
        chi_(chi), rg_(rg),
        c1_(1.0), c2_(0.0),
        w1_(1.0), w2_(0.0), w3_(0.0) {}

  SAXSResult(int number, float chi, bool filtered, float rg, float c1, float c2,
             IMP::algebra::Transformation3D transformation)
      : Result(number, chi, filtered, 0.0, transformation),
        chi_(chi), rg_(rg),
        c1_(c1), c2_(c2),
        w1_(1.0), w2_(0.0), w3_(0.0) {}

  SAXSResult(int number, float chi, bool filtered, float rg, float c1, float c2,
             float w1, float w2, float w3,
             IMP::algebra::Transformation3D transformation)
      : Result(number, chi, filtered, 0.0, transformation),
        chi_(chi), rg_(rg),
        c1_(c1), c2_(c2),
        w1_(w1), w2_(w2), w3_(w3) {}

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
    // weights
    s.width(6);
    s << p.w1_ << " | ";
    s.width(6);
    s << p.w2_ << " | ";
    s.width(6);
    s << p.w3_ << " | ";
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
    out << "     # | Chi    |filter| Zscore | Rg     | c1     | c2     | w1   | w2   | w3   |"
           "Transformation" << std::endl;
  }

 protected:
  float chi_;
  float rg_;
  float c1_, c2_;  // foxs fitting parameters
  float w1_, w2_, w3_; // weights
};

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_SAXS_RESULT_H */
