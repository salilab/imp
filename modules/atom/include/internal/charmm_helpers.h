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
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Fragment.h>
#include <IMP/atom/Residue.h>

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

//! A visitor to get chains of connected residues from a Hierarchy
/** When called via visit_depth_first(), it calls inserter.add_chain()
    for each such set. */
template<class Inserter>
class TopologyVisitor {
public:
  TopologyVisitor(Inserter &inserter) : inserter_(inserter), in_chain_(false) {}

  bool operator()(Hierarchy p) {
    Chain chain = p.get_as_chain();
    Fragment fragment = p.get_as_fragment();
    Residue residue = p.get_as_residue();
    if (residue) {
      if (last_fragment_) {
        add_chain();
        last_fragment_ = Fragment();
      }
      // assume that all residues not wrapped in a Fragment are connected
      fragment_residues_.push_back(p);
      // don't look at atoms
      return false;
    } else if (fragment) {
      // If existing residues are not from a fragment, or they're in a fragment
      // that is not connected to this one, write them out
      if (!last_fragment_ || !fragments_adjacent(last_fragment_, fragment)) {
        add_chain();
      }
      last_fragment_ = fragment;
      Hierarchies r = get_by_type(p, RESIDUE_TYPE);
      fragment_residues_.insert(fragment_residues_.end(), r.begin(), r.end());
      // We already looked at child Residues, so don't look at them again
      return false;
    } else if (chain && !in_chain_) {
      // Flush old chain (e.g. Fragments not part of a Chain)
      add_chain();
      // Recurse into this chain
      in_chain_ = true; // prevent infinite recursion
      core::visit_depth_first(p, *this);
      in_chain_ = false;
      // Flush new chain
      add_chain();
      // We already looked at the chain, so don't need to do it again
      return false;
    } else {
      return true;
    }
  }

  void add_chain() {
    // Ignore empty chains
    if (fragment_residues_.size() > 0) {
      inserter_.add_chain(fragment_residues_);
      fragment_residues_.clear();
    }
  }

private:
  //! Return true iff two fragments are adjacent in sequence
  /** This is taken to be the case only if the last residue in the first
      fragment and the first residue in the second fragment are contiguous */
  bool fragments_adjacent(Fragment f1, Fragment f2) {
    Ints p1 = f1.get_residue_indexes();
    Ints p2 = f2.get_residue_indexes();
    if (p1.size() > 0 && p2.size() > 0) {
      return p1[p1.size() - 1] + 1 == p2[0];
    } else {
      return false;
    }
  }

  Inserter &inserter_;
  Hierarchies fragment_residues_;
  Fragment last_fragment_;
  bool in_chain_;
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_CHARMM_HELPERS_H */
