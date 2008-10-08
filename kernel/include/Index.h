/**
 *  \file Index.h   \brief Classes for typechecked indices.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INDEX_H
#define IMP_INDEX_H

#include "macros.h"
#include "exception.h"

IMP_BEGIN_NAMESPACE

//! A simple class for defining typechecked indices.
/** Non-default instances of Index are well ordered.
    A defaultly constructed Index can be compared for equality, but not ordered.
 */
template <class L>
class Index
{
public:
  typedef Index<L> This;
  //! Construct an index from a nonnegative int
  Index(unsigned int i): i_(i) {
    IMP_check(i >= 0, "Index initializer must be positive. " << i << " is not.",
              IndexException);
  }
  //! Construct a default index
  /** This can be used as a sentinal value */
  Index(): i_(-1) {}

  //! Return an integer for this index
  /** The integer is unique within the container */
  unsigned int get_index() const {
    IMP_check(i_ >= 0, "get_index() called on defaultly constructed Index",
              ValueException);
    return i_;
  }
  void show(std::ostream &out) const {
    if (!is_default()) {
      out << "(" << i_ << ")";
    } else {
      out << "(Invalid)";
    }
  }
  IMP_COMPARISONS_1(i_)

#ifndef SWIG
  //! This should be protected
  /**
     \note Really we only want this accessible from the iterators in
     ModelData, but I can't get that to work. Don't use it.
   */
  void operator++() {
    ++i_;
  }
#endif
protected:
  bool is_default() const {
    return i_==-1;
  }

  int i_;
};


IMP_OUTPUT_OPERATOR_1(Index)

IMP_END_NAMESPACE

#endif  /* IMP_INDEX_H */
