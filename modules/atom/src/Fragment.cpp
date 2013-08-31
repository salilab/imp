/**
 *  \file Fragment.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Fragment.h"
#include <algorithm>

IMPATOM_BEGIN_NAMESPACE

void Fragment::show(std::ostream &out) const {
  out << "Fragment: ";
  IntPairs ps = get_residue_index_ranges();
  for (unsigned int i = 0; i < ps.size(); ++i) {
    if (ps[i].first != ps[i].second - 1) {
      out << "[" << ps[i].first << ", " << ps[i].second << ") ";
    } else {
      out << ps[i].first << " ";
    }
  }
}

Ints Fragment::get_residue_indexes() const {
  IntPairs ranges = get_residue_index_ranges();
  Ints ret;
  for (unsigned int i = 0; i < ranges.size(); ++i) {
    for (int j = ranges[i].first; j < ranges[i].second; ++j) {
      ret.push_back(j);
    }
  }
  return ret;
}

IntsKey Fragment::get_begins_key() {
  static IntsKey k("fragment begins");
  return k;
}
IntsKey Fragment::get_ends_key() {
  static IntsKey k("fragment ends");
  return k;
}
IntKey Fragment::get_marker_key() {
  static IntKey k("fragment marker");
  return k;
}

bool Fragment::get_contains_residue(int ri) const {
  IntPairs all = get_residue_index_ranges();
  for (unsigned int i = 0; i < all.size(); ++i) {
    if (ri >= all[i].first && ri < all[i].second) return true;
  }
  return false;
}

IntPairs Fragment::get_residue_index_ranges() const {
  if (!get_model()->get_has_attribute(get_begins_key(), get_particle_index())) {
    return IntPairs();
  }
  Ints begins =
      get_model()->get_attribute(get_begins_key(), get_particle_index());
  Ints ends = get_model()->get_attribute(get_ends_key(), get_particle_index());
  IMP_INTERNAL_CHECK(begins.size() == ends.size(),
                     "The fragment residues are corrupted.");
  IntPairs ret(begins.size());
  for (unsigned int i = 0; i < ret.size(); ++i) {
    ret[i] = IntPair(begins[i], ends[i]);
  }
  return ret;
}

void Fragment::set_residue_indexes(Model *m,
                                   kernel::ParticleIndex pi, const IntPairs &ris) {
  Ints begins(ris.size());
  Ints ends(ris.size());
  for (unsigned int i = 0; i < ris.size(); ++i) {
    begins[i] = ris[i].first;
    ends[i] = ris[i].second;
    IMP_USAGE_CHECK(ris[i].first < ris[i].second,
                    "Bad range for residue indexes");
  }
  if (begins.size() > 0) {
    if (m->get_has_attribute(get_begins_key(), pi)) {
      m->set_attribute(get_begins_key(), pi, begins);
      m->set_attribute(get_ends_key(), pi, ends);
    } else {
      m->add_attribute(get_begins_key(), pi, begins);
      m->add_attribute(get_ends_key(), pi, ends);
    }
  } else {
    if (m->get_has_attribute(get_begins_key(), pi)) {
      m->remove_attribute(get_begins_key(), pi);
      m->remove_attribute(get_ends_key(), pi);
    }
  }
}

void Fragment::set_residue_indexes(Model *m,
                                   kernel::ParticleIndex pi, Ints o) {
  if (o.empty()) {
    set_residue_indexes(m, pi, IntPairs());
    return;
  }
  std::sort(o.begin(), o.end());
  o.erase(std::unique(o.begin(), o.end()), o.end());
  IntPairs pairs;
  int begin = 0;
  for (unsigned int i = 1; i < o.size(); ++i) {
    if (o[i] != o[i - 1] + 1) {
      pairs.push_back(IntPair(o[begin], o[i - 1] + 1));
      begin = i;
    }
  }
  pairs.push_back(IntPair(o[begin], o.back() + 1));
  set_residue_indexes(m, pi, pairs);
  using IMP::operator<< ;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i = 0; i < o.size(); ++i) {
      IMP_INTERNAL_CHECK(Fragment(m, pi).get_contains_residue(o[i]),
                         "Residue index not found after addition: "
                             << o << " became " << pairs);
    }
  }
}

IMPATOM_END_NAMESPACE
