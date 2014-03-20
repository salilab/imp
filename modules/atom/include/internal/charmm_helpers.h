/**
 *  \file charmm_helpers.h
 *  \brief Helpers for the CHARMM forcefield support.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_INTERNAL_CHARMM_HELPERS_H
#define IMPATOM_INTERNAL_CHARMM_HELPERS_H

#include <IMP/atom/atom_config.h>

#include <IMP/base_types.h>
#include <IMP/kernel/Particle.h>

IMPATOM_BEGIN_NAMESPACE
class CHARMMParameters;
IMPATOM_END_NAMESPACE

IMPATOM_BEGIN_INTERNAL_NAMESPACE

class CHARMMBondNames {
  std::string a_, b_;

 public:
  CHARMMBondNames(std::string a, std::string b) : a_(a), b_(b) {
    // store atom type names sorted so AB == BA
    if (a_ > b_) {
      std::swap(a_, b_);
    }
  }
  inline bool operator==(const CHARMMBondNames &other) const {
    return a_ == other.a_ && b_ == other.b_;
  }
  inline bool operator<(const CHARMMBondNames &other) const {
    return a_ + b_ < other.a_ + other.b_;
  }
};

class CHARMMAngleNames {
  std::string a_, b_, c_;

 public:
  CHARMMAngleNames(std::string a, std::string b, std::string c)
      : a_(a), b_(b), c_(c) {
    // store atom type names sorted so ABC == CBA (but not other permutations)
    if (a_ > c_) {
      std::swap(a_, c_);
    }
  }
  inline bool operator==(const CHARMMAngleNames &other) const {
    return a_ == other.a_ && b_ == other.b_ && c_ == other.c_;
  }
  inline bool operator<(const CHARMMAngleNames &other) const {
    return a_ + b_ + c_ < other.a_ + other.b_ + other.c_;
  }
};

class CHARMMDihedralNames {
  std::string a_, b_, c_, d_;

  static inline int match(std::string ref, std::string cmp,
                          bool allow_wildcards) {
    if (ref == cmp) {
      return 0;
    } else if (ref == "X" && allow_wildcards) {
      return 1;
    } else {
      return MISMATCH;
    }
  }

 public:
  const static int MISMATCH = 100;
  CHARMMDihedralNames(std::string a, std::string b, std::string c,
                      std::string d)
      : a_(a), b_(b), c_(c), d_(d) {}

  inline int match(const CHARMMDihedralNames &other,
                   bool allow_wildcards) const {
    int n_match = match(a_, other.a_, allow_wildcards) +
                  match(b_, other.b_, allow_wildcards) +
                  match(c_, other.c_, allow_wildcards) +
                  match(d_, other.d_, allow_wildcards);
    int n_match_rev = match(a_, other.d_, allow_wildcards) +
                      match(b_, other.c_, allow_wildcards) +
                      match(c_, other.b_, allow_wildcards) +
                      match(d_, other.a_, allow_wildcards);
    return std::min(n_match, n_match_rev);
  }
};

//! Create a Dihedral on the given kernel::Particles, and add it to the list
IMPATOMEXPORT void add_dihedral_to_list(
    const CHARMMParameters *param, kernel::Particle *p1, kernel::Particle *p2,
    kernel::Particle *p3, kernel::Particle *p4, kernel::Particles &ps);

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_CHARMM_HELPERS_H */
