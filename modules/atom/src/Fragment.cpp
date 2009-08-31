/**
 *  \file Fragment.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/atom/Fragment.h"
#include <algorithm>


IMPATOM_BEGIN_NAMESPACE

Fragment::Traits Fragment::btraits_("fragment.begins");
Fragment::Traits Fragment::etraits_("fragment.ends");

Fragment::~Fragment(){}


void Fragment::show(std::ostream &out) const {
  out << "Fragment: ";
  for (unsigned int i=0; i< get_number_of_residue_begins(); ++i) {
    if (get_residue_end(i) != get_residue_begin(i)+1) {
      out << "[" << get_residue_begin(i)
          << ", " << get_residue_end(i) << ") ";
    } else {
      out << get_residue_begin(i) << " ";
    }
  }
}

void Fragment::add_residue_indexes(int begin, int end) {
  add_residue_end(end);
  add_residue_begin(begin);
}

Ints Fragment::get_residue_indexes() const {
  Ints ret;
  for (unsigned int i=0; i< get_number_of_residue_begins(); ++i) {
    for (int j= get_residue_begin(i); j < get_residue_end(i);
         ++j) {
      ret.push_back(j);
    }
  }
  return ret;
}

void Fragment::set_residue_indexes(Ints o) {
  clear_residue_ends();
  clear_residue_begins();
  std::sort(o.begin(), o.end());
  o.erase(std::unique(o.begin(), o.end()), o.end());
  int begin=0;
  for (unsigned int i=1; i< o.size(); ++i) {
    if (o[i] != o[i-1]+1) {
      add_residue_indexes(o[begin], o[i-1]+1);
      begin=i;
    }
  }
  add_residue_indexes(o[begin], o.back()+1);
  IMP_IF_CHECK(EXPENSIVE) {
    Ints check(residue_indexes_begin(),
               residue_indexes_end());
    IMP_assert(check.size() == o.size(),
               "Didn't start and end with the name number of indexes "
               << check.size() << " " << o.size());
    for (unsigned int i=0; i< check.size(); ++i) {
      IMP_assert(check[i] == o[i], "Expected residue " << check[i]
                 << " got residue " << o[i]);
    }
  }
}

IMPATOM_END_NAMESPACE
