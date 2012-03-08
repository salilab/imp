/**
 *  \file RestraintSet.h     \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_SET_H
#define IMPKERNEL_RESTRAINT_SET_H

#include "kernel_config.h"
#include "declare_RestraintSet.h"

IMP_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace {
  template <class It>
  void get_restraints_internal(It b, It e,
                               RestraintsTemp &ret) {
    for (It c=b; c!= e; ++c) {
      Restraint *cur= *c;
      RestraintSet *rs=dynamic_cast<RestraintSet*>(cur);
      if (rs) {
        get_restraints_internal(rs->restraints_begin(),
                                rs->restraints_end(), ret);
      } else {
        ret.push_back(cur);
      }
    }
  }
}
#endif

template <class It>
inline RestraintsTemp get_restraints(It b, It e) {
  RestraintsTemp ret;
  get_restraints_internal(b,e, ret);
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_RESTRAINT_SET_H */
