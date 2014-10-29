/**
 * \file IMP/integrative_docking/CrossLinkingResult.h
 * \brief A class that stores one docking model with cross linking scores
 *
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_CROSS_LINKING_RESULT_H
#define IMPINTEGRATIVE_DOCKING_CROSS_LINKING_RESULT_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include "Result.h"

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

class CrossLinkingResult : public Result {
 public:
  CrossLinkingResult(int number, float score, bool filtered,
                     int unsatisfied_num,
                     IMP::algebra::Transformation3D transformation)
      : Result(number, score, filtered, 0.0, transformation),
        unsatisfied_num_(unsatisfied_num) {}

  friend std::ostream& operator<<(std::ostream& s,
                                  const CrossLinkingResult& p) {
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
    s << p.unsatisfied_num_ << " | ";
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
    out << "     # | Score  |filter| Zscore |UnsatNum| Transformation "
        << std::endl;
  }

 protected:
  int unsatisfied_num_;  // number of unsatisfied cross links
};

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_CROSS_LINKING_RESULT_H */
