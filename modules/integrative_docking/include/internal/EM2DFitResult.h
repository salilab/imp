/**
 *  \file IMP/integrative_docking/EM2DFitResult.h
 *  \brief A class that stores one docking model with the related scores
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_EM2DFIT_RESULT_H
#define IMPINTEGRATIVE_DOCKING_EM2DFIT_RESULT_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include "Result.h"

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

class EM2DFitResult : public Result {
 public:
  EM2DFitResult(int number, float cc, bool filtered,
            const IMP::algebra::Transformation3D& transformation)
      : Result(number, cc, filtered, 0.0, transformation), cc_(cc) {}

  // access
  float get_cc_score() const { return cc_; }
  float get_score() const { return 1 - cc_; }

  // IO
  friend std::ostream& operator<<(std::ostream& s, const EM2DFitResult& p) {
    s.width(6);
    s << p.number_ << " | ";
    s.precision(3);
    s.width(6);
    s << p.get_score();
    if (p.filtered_)
      s << " |  -   | ";
    else
      s << " |  +   | ";
    s.width(6);
    s << p.z_score_ << " | ";
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
    out << "     # | 1-CC   |filter| Zscore | Transformation" << std::endl;
  }

 private:
  float cc_;  // cross correlation score
};

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_EM2DFIT_RESULT_H */
