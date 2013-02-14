/**
 *  \file FitResult.h   \brief A class that stores one docking model
 * with the related scores
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_FIT_RESULT_H
#define IMP_FIT_RESULT_H

#include <IMP/algebra/Transformation3D.h>

class FitResult {
public:
  FitResult(int number, float score, bool filtered, float cc,
            float envelope_score,
            const IMP::algebra::Transformation3D& ligand_trans,
            const IMP::algebra::Transformation3D& map_trans) :
    number_(number), score_(score), filtered_(filtered), z_score_(0.0), cc_(cc),
    envelope_score_(envelope_score),
    ligand_trans_(ligand_trans), map_trans_(map_trans) {}

  // access
  int get_number() const { return number_; }
  float get_score() const { return score_; }
  bool is_filtered() const { return filtered_; }
  float get_cc_score() const { return cc_; }
  float get_envelope_score() const { return envelope_score_; }

  const IMP::algebra::Transformation3D& get_ligand_trans() const
  { return ligand_trans_; }
  const IMP::algebra::Transformation3D& get_map_trans() const
  { return map_trans_; }

  // modifiers
  void set_z_score(float z_score) { z_score_ = z_score; }

  // IO
  friend std::ostream& operator<<(std::ostream& s, const FitResult& p) {
    s.width(6); s << p.number_ << " | ";
    s.precision(3); s.width(8); s << p.get_score();
    if(p.filtered_) s << " |  -   | "; else s << " |  +   | ";
    s.width(6); s << p.z_score_ << " | ";
    s.width(6); s << p.cc_ << " | ";
    s.width(8); s << p.envelope_score_ << " | ";
    IMP::algebra::FixedXYZ euler_angles =
      IMP::algebra::get_fixed_xyz_from_rotation(p.map_trans_.get_rotation());
    IMP::algebra::Vector3D translation = p.map_trans_.get_translation();
    s.precision(4);
    s << euler_angles.get_x() << " "
      << euler_angles.get_y() << " "
      << euler_angles.get_z() << " "
      << translation[0] << " "
      << translation[1] << " "
      << translation[2] << " | ";
    euler_angles =
      IMP::algebra::get_fixed_xyz_from_rotation(p.ligand_trans_.get_rotation());
    translation = p.ligand_trans_.get_translation();
    s.precision(4);
    s << euler_angles.get_x() << " "
      << euler_angles.get_y() << " "
      << euler_angles.get_z() << " "
      << translation[0] << " " << translation[1] << " " << translation[2];
    return s;
  }

  static void print_header(std::ostream& out) {
    out << "     # | Score    |filter| Zscore |  CC    | Escore   | \
Map transformation                          | Ligand Transformation"
        << std::endl;
  }

private:
  int number_;
  float score_; // score
  bool filtered_;
  float z_score_;
  float cc_; // cross correlation score
  float envelope_score_;
  // docking transformation that brings a ligand to receptor
  IMP::algebra::Transformation3D ligand_trans_;
  // transformation that bring receptor to the density map
  IMP::algebra::Transformation3D map_trans_;
};

struct CompareFitResults {
  bool operator()(const FitResult& r1, const FitResult& r2) const {
    if (r1.get_score() > r2.get_score()) return true;
    return false;
  }
};

#endif /* IMP_FIT_RESULT_H */
