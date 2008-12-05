/**
 *  \file key_helpers.h    \brief helpers for declaring keys.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_KEY_HELPERS_H
#define IMP_INTERNAL_KEY_HELPERS_H

#include "../IMP_config.h"
#include <vector>
#include <map>

IMP_BEGIN_INTERNAL_NAMESPACE
/** \internal The data concerning keys.
  */
struct IMPDLLEXPORT KeyData
{
  typedef std::map<std::string, int> Map;
  typedef std::vector<std::string> RMap;

  void show(std::ostream &out= std::cout) const;
  KeyData();
  void assert_is_initialized() const;
  unsigned int add_key(std::string str) {
    unsigned int i= map_.size();
    map_[str]=i;
    rmap_.push_back(str);
    return i;
  }

  const Map &get_map() const {return map_;}
  const RMap &get_rmap() const {return rmap_;}

private:
  double heuristic_;
  Map map_;
  RMap rmap_;
};

IMPDLLEXPORT extern std::map<unsigned int, KeyData> key_data;

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_KEY_HELPERS_H */
