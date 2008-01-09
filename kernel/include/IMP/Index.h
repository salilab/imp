/**
 *  \file Index.h   \brief Classes for typechecked indices.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_INDEX_H
#define __IMP_INDEX_H

#include "log.h"
#include "utility.h"

namespace IMP
{

//! A simple class for defining typechecked indices.
/** Non-default instances of Index are well ordered.
    A defaultly constructed Index can be compared for equality, but not ordered.
 */
template <class L>
class Index
{
public:
  typedef Index<L> This;
  Index(int i): i_(i) {
    IMP_check(i >= 0, "Index initializer must be positive. " << i << " is not.",
              ErrorException());
  }
  Index(): i_(-1) {}
  unsigned int get_index() const {
    IMP_check(i_ >= 0, "get_index() called on defaultly constructed Index",
              ErrorException());
    return i_;
  }
  std::ostream &show(std::ostream &out) const {
    if (!is_default()) {
      out << "(" << i_ << ")";
    } else {
      out << "(Invalid)";
    }
    return out;
  }
  IMP_COMPARISONS_1(i_)

private:
  bool is_default() const {
    return i_==-1;
  }

  int i_;
};


IMP_OUTPUT_OPERATOR_1(Index)


} // namespace IMP

#endif  /* __IMP_INDEX_H */
