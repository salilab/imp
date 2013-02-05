/**
 *  \file key_helpers.h    \brief helpers for declaring keys.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_KEY_HELPERS_H
#define IMPKERNEL_INTERNAL_KEY_HELPERS_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/exception.h>
#include <IMP/base/map.h>
#include <IMP/base/Vector.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
/** \internal The data concerning keys.
  */
struct IMPKERNELEXPORT KeyData
{
  typedef IMP::base::map<std::string, int> Map;
  typedef IMP::base::Vector<std::string> RMap;

  void show(std::ostream &out= std::cout) const;
  KeyData();
  void assert_is_initialized() const;
  unsigned int add_key(std::string str) {
    unsigned int i= rmap_.size();
    map_[str]=i;
    rmap_.push_back(str);
    return i;
  }
  unsigned int add_alias(std::string str, unsigned int i) {
    IMP_INTERNAL_CHECK(rmap_.size() > i, "The aliased key doesn't exist");
    map_[str]=i;
    return i;
  }
  const Map &get_map() const {return map_;}
  const RMap &get_rmap() const {return rmap_;}

private:
  double heuristic_;
  Map map_;
  RMap rmap_;
};

IMPKERNELEXPORT KeyData& get_key_data(unsigned int index);

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_KEY_HELPERS_H */
