/**
 *  \file SAXSResult.h   \brief A class that stores one docking model
 * with SAXS related scores
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_SAXS_RESULT_H
#define IMP_SAXS_RESULT_H

#include <IMP/algebra/Transformation3D.h>

#include <iostream>

class SAXSResult {
public:
  SAXSResult(int number, float chi, bool filtered, float rg,
             IMP::algebra::Transformation3D transformation) :
    number_(number), chi_(chi), filtered_(filtered), z_score_(0.0),
    rg_(rg), transformation_(transformation) {}

  SAXSResult(int number, float chi, bool filtered, float rg,
             float c1, float c2, IMP::algebra::Transformation3D transformation):
    number_(number), chi_(chi), filtered_(filtered), z_score_(0.0),
    rg_(rg), c1_(c1), c2_(c2), transformation_(transformation) {}

  float get_chi() const { return chi_; }
  bool is_filtered() const { return filtered_; }

  void set_z_score(float z_score) { z_score_ = z_score; }

  friend std::ostream& operator<<(std::ostream& s, const SAXSResult& p) {
    s.width(6); s << p.number_ << " | ";
    s.precision(3); s.width(6); s << p.chi_;
    if(p.filtered_) s << " |  -   | "; else s << " |  +   | ";
    s.width(6); s << p.z_score_ << " | ";
    s.width(6); s << p.rg_ << " | ";
    s.width(6); s << p.c1_ << " | ";
    s.width(6); s << p.c2_ << " | ";
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
   "     # | Chi    |filter| Zscore | Rg     | c1     | c2     | Transformation"
        << std::endl;
  }

protected:
  int number_;
  float chi_;
  bool filtered_;
  float z_score_;
  float rg_;
  float c1_,c2_; // foxs fitting parameters
  IMP::algebra::Transformation3D transformation_;
};

#endif /* IMP_SAXS_RESULT_H */
