/**
 * \file RotamerLibrary.cpp \brief Access to RotamerLibrary.
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <IMP/rotamer/RotamerLibrary.h>

IMPROTAMER_BEGIN_NAMESPACE

RotamerLibrary::RotamerLibrary(unsigned angle_step)
    : IMP::base::Object("RotamerLibrary%1%"), angle_step_(angle_step) {
  IMP_USAGE_CHECK(angle_step_ > 0 && angle_step_ <= 360,
                  "angle step must be within (0, 360]");
  rotamers_by_backbone_size_ = 2 * 180 / angle_step_ + 1;
}

// The data corresponding to the backbone angles phi and psi is stored
// in a vector of buckets of size angle_step. backbone_angle_to_index gives
// the index in the bucket vector where the data is located
unsigned RotamerLibrary::backbone_angle_to_index(float phi, float psi) const {
  int i_phi = int(phi) % 360, i_psi = int(psi) % 360;
  if (i_phi < 0) i_phi += 360;
  if (i_psi < 0) i_psi += 360;
  // 0 <= i_phi < 360, 0 <= i_psi < 360
  i_phi /= angle_step_;
  i_psi /= angle_step_;
  return i_phi * 360 / angle_step_ + i_psi;
}

namespace {

// RotamerSortComparator is used to sort the angle data based on the probability
// (elements of higher probability precede the elements of lower probability).
// In addition it is used (together with RotamerEqualComparator) to remove exact
// duplicates that occur in the library files
struct RotamerSortComparator {
  bool operator()(const RotamerAngleTuple &r1,
                  const RotamerAngleTuple &r2) const {
    // r1 precedes r2 if r1 has higher probability
    if (r1.get_probability() != r2.get_probability()) {
      return r1.get_probability() > r2.get_probability();
    }
    // the rest of comparison impose some arbitrary ordering
    if (r1.get_chi1() != r2.get_chi1()) {
      return r1.get_chi1() > r2.get_chi1();
    }
    if (r1.get_chi2() != r2.get_chi2()) {
      return r1.get_chi2() > r2.get_chi2();
    }
    if (r1.get_chi3() != r2.get_chi3()) {
      return r1.get_chi3() > r2.get_chi3();
    }
    return r1.get_chi4() > r2.get_chi4();
  }
};

// this comparator is used with std::unique to get rid of repetitions
struct RotamerEqualComparator {
  bool operator()(const RotamerAngleTuple &r1,
                  const RotamerAngleTuple &r2) const {
    RotamerSortComparator precedes;
    // r1 equals r2 if r1 does not precede r2 and r2 does not precede r1
    return !precedes(r1, r2) && !precedes(r2, r1);
  }
};
}

void RotamerLibrary::read_library_file(const std::string &lib_file_name) {
  std::ifstream rotf(lib_file_name.c_str());

  IMP_USAGE_CHECK(rotf, "Cannot open file " + lib_file_name);

  std::string line;
  while (std::getline(rotf, line)) {
    std::string::size_type p = line.find('#');
    if (p != std::string::npos) line.erase(p);
    std::string res_name;
    float phi, psi;
    int count, r1, r2, r3, r4;
    float probability;
    float chi1, chi2, chi3, chi4;
    std::istringstream line_str(line);
    if (line_str >> res_name >> phi >> psi >> count >> r1 >> r2 >> r3 >> r4 >>
        probability >> chi1 >> chi2 >> chi3 >> chi4) {
      IMP::atom::ResidueType residue(res_name);
      // the top level vector is indexed by the index of the residue
      unsigned res_index = residue.get_index();
      if (res_index >= library_.size()) {
        library_.resize(res_index + 1);
      }
      // find the location that corresponds to phi and psi backbone angles
      unsigned angle_index = backbone_angle_to_index(phi, psi);
      if (library_[res_index].empty()) {
        library_[res_index]
            .resize(rotamers_by_backbone_size_ * rotamers_by_backbone_size_);
      }
      library_[res_index][angle_index]
          .push_back(RotamerAngleTuple(chi1, chi2, chi3, chi4, probability));
    }
  }
  for (size_t i = 0; i != library_.size(); ++i)
    for (size_t j = 0; j != library_[i].size(); ++j) {
      RotamerAngleTuples &rr = library_[i][j];
      // sort and remove duplicates
      std::sort(rr.begin(), rr.end(), RotamerSortComparator());
      rr.erase(std::unique(rr.begin(), rr.end(), RotamerEqualComparator()),
               rr.end());
    }
}

RotamerLibrary::RotamerRange RotamerLibrary::get_rotamers_fast(
    IMP::atom::ResidueType residue, float phi, float psi,
    float probability_thr) const {
  unsigned res_index = residue.get_index();
  if (res_index >= library_.size()) {
    return boost::make_iterator_range(RotamerIterator(), RotamerIterator());
  }
  unsigned angle_index = backbone_angle_to_index(phi, psi);
  if (angle_index >= library_[res_index].size()) {
    return boost::make_iterator_range(RotamerIterator(), RotamerIterator());
  }
  RotamerIterator rot_begin = library_[res_index][angle_index].begin();
  RotamerIterator rot_end = rot_begin;
  double prob_so_far = 0;
  while (prob_so_far < probability_thr &&
         rot_end != library_[res_index][angle_index].end()) {
    prob_so_far += rot_end->get_probability();
    ++rot_end;
  }
  return boost::make_iterator_range(rot_begin, rot_end);
}

RotamerAngleTuples RotamerLibrary::get_rotamers(IMP::atom::ResidueType residue,
                                                float phi, float psi,
                                                float probability_thr) const {
  RotamerRange r = get_rotamers_fast(residue, phi, psi, probability_thr);
  return RotamerAngleTuples(r.begin(), r.end());
}

IMPROTAMER_END_NAMESPACE
