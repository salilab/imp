/**
 *  \file Index.h   \brief Classes for typechecked indices.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_INDEX_H
#define __IMP_INDEX_H

#include "log.h"

namespace IMP {

//! A simple class for defining typechecked indices. 
template <class L>
class Index {
public:
  Index(int i=-1): i_(i) {
  }
  unsigned int get_index() const {
    IMP_check(i_ >=0, "get_index() called on defaultly constructed Index",
	      ErrorException());
    return i_;
  }
  std::ostream &show(std::ostream &out) const {
    out << "(" << i_ << ")";
    return out;
  }
  bool operator==(const Index<L> &o) const {return i_==o.i_;}
  bool operator>(const Index<L> &o) const {return i_ > o.i_;}
  bool operator<(const Index<L> &o) const {return i_ < o.i_;}
  bool operator!=(const Index<L> &o) const {return i_!=o.i_;}
  bool operator>=(const Index<L> &o) const {return i_ >= o.i_;}
  bool operator<=(const Index<L> &o) const {return i_ <= o.i_;}
private:
  int i_;
};

template <class L>
std::ostream &operator<<(std::ostream &out, const Index<L> &i) {
  return i.show(out);
}

} // namespace IMP

#endif  /* __IMP_INDEX_H */
