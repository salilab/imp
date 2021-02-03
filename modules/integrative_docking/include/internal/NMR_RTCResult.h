/**
 * \file IMP/integrative_docking/NMR_RTCResult.h
 * \brief A class that stores one docking model with NMR_RTC related scores
 *
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_NMR_RTC_RESULT_H
#define IMPINTEGRATIVE_DOCKING_NMR_RTC_RESULT_H

#include <IMP/integrative_docking/integrative_docking_config.h>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

class NMR_RTCResult {
 public:
  NMR_RTCResult(int number, float score, bool filtered, int diff_num,
                IMP::algebra::Transformation3D transformation)
      : number_(number),
        score_(score),
        filtered_(filtered),
        z_score_(0.0),
        diff_num_(diff_num),
        transformation_(transformation) {}

  float get_score() const { return score_; }
  bool is_filtered() const { return filtered_; }

  void set_z_score(float z_score) { z_score_ = z_score; }

  friend std::ostream& operator<<(std::ostream& s, const NMR_RTCResult& p) {
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
    s << p.diff_num_ << " | ";
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
    out << "     # | Score  |filter| Zscore | DiffNum| Transformation "
        << std::endl;
  }

 protected:
  int number_;
  float score_;
  bool filtered_;
  float z_score_;
  int diff_num_;  // number of residues that differ from exp. residue content
  IMP::algebra::Transformation3D transformation_;
};

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_NMR_RTC_RESULT_H */
