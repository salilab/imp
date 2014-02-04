/**
 *  \file FitResult.h   \brief A class that stores one docking model
 * with the related scores
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_FIT_RESULT_H
#define IMP_FIT_RESULT_H

#include "../../lib/Result.h"

class FitResult : public Result {
 public:
  FitResult(int number, float cc, bool filtered,
            const IMP::algebra::Transformation3D& transformation)
      : Result(number, cc, filtered, 0.0, transformation), cc_(cc) {}

  // access
  float get_cc_score() const { return cc_; }
  float get_score() const { return 1 - cc_; }

  // IO
  friend std::ostream& operator<<(std::ostream& s, const FitResult& p) {
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

#endif /* IMP_FIT_RESULT_H */
