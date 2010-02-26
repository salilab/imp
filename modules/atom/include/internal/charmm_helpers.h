/**
 *  \file charmm_helpers.h
 *  \brief Helpers for the CHARMM forcefield support.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_INTERNAL_CHARMM_HELPERS_H
#define IMPATOM_INTERNAL_CHARMM_HELPERS_H

#include "../config.h"

#include <IMP/base_types.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

class CHARMMBondNames
{
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

class CHARMMAngleNames
{
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

IMPATOM_END_INTERNAL_NAMESPACE

#endif  /* IMPATOM_INTERNAL_CHARMM_HELPERS_H */
