/**
 *  \file Hierarchy.cpp   \brief Decorator for helping deal with
 *                                        a hierarchy.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/Hierarchy.h>

#include <sstream>

IMPCORE_BEGIN_NAMESPACE

const HierarchyTraits &Hierarchy::get_default_traits() {
  static HierarchyTraits ret("hierarchy");
  return ret;
}

HierarchyTraits::HierarchyTraits(std::string name) {
  parent_ = ParticleIndexKey((name + "_parent").c_str());
  children_ = ParticleIndexesKey((name + "_children").c_str());
}

void Hierarchy::show(std::ostream &out) const { out << "Hierarchy"; }

int Hierarchy::get_child_index() const {
  if (!get_parent())
    return -1;
  else {
    ParticleIndex pi = get_model()->get_attribute(
        get_decorator_traits().get_parent_key(), get_particle_index());
    const ParticleIndexes &pis = get_model()->get_attribute(
        get_decorator_traits().get_children_key(), pi);
    int ret =
        std::find(pis.begin(), pis.end(), get_particle_index()) - pis.begin();
    IMP_INTERNAL_CHECK(Hierarchy(get_model(), pi, get_decorator_traits())
                               .get_child(ret) == *this,
                       "ith child isn't");
    return ret;
  }
}

namespace {

struct MHDMatchingLeaves {
  HierarchyTraits traits_;
  MHDMatchingLeaves(HierarchyTraits tr) : traits_(tr) {}
  bool operator()(Particle *p) const {
    Hierarchy mhd(p, traits_);
    return mhd.get_number_of_children() == 0;
  }
};

}  // namespace

GenericHierarchies get_leaves(Hierarchy mhd) {
  GenericHierarchies out;
  gather(mhd, MHDMatchingLeaves(mhd.get_decorator_traits()),
         std::back_inserter(out));
  return out;
}

namespace {

struct MHDNotMatchingLeaves {
  HierarchyTraits traits_;
  MHDNotMatchingLeaves(HierarchyTraits tr) : traits_(tr) {}
  bool operator()(Particle *p) const {
    Hierarchy mhd(p, traits_);
    return mhd.get_number_of_children() != 0;
  }
};

}  // namespace

GenericHierarchies get_internal(Hierarchy mhd) {
  GenericHierarchies out;
  gather(mhd, MHDNotMatchingLeaves(mhd.get_decorator_traits()),
         std::back_inserter(out));
  return out;
}

namespace {

struct MHDMatchingAll {
  bool operator()(Particle *) const { return true; }
};

}  // namespace

GenericHierarchies get_all_descendants(Hierarchy mhd) {
  GenericHierarchies out;
  gather(mhd, MHDMatchingAll(), std::back_inserter(out));
  return out;
}

IMPCORE_END_NAMESPACE
