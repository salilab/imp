/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/decorator/alternatives.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/sequence.h"
#include <numeric>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace decorator {

namespace {
double get_resolution_metric(double a, double b) {
  if (a < b) std::swap(a, b);
  return a / b - 1;
}

std::pair<double, bool> get_resolution_impl(NodeConstHandle root,
                                              IntermediateParticleFactory ipcf,
                                              GaussianParticleFactory gpf) {
  std::pair<double, bool> ret(std::numeric_limits<double>::max(), false);
  for(NodeConstHandle ch : root.get_children()) {
    std::pair<double, bool> cur = get_resolution_impl(ch, ipcf, gpf);
    ret.first = std::min(ret.first, cur.first);
    ret.second = ret.second || cur.second;
  }
  if (!ret.second) {
    if (ipcf.get_is(root)) {
      ret.first = ipcf.get(root).get_radius();
      ret.second = true;
    } else if (gpf.get_is(root)) {
      Vector3 sdfs = gpf.get(root).get_variances();
      ret.first = std::accumulate(sdfs.begin(), sdfs.end(), 0.0) / 3.0;
      ret.second = true;
    }
  }
  return ret;
}
}

AlternativesFactory::AlternativesFactory(FileHandle fh)
    : cat_(fh.get_category("alternatives")),
      types_key_(fh.get_key<IntsTag>(cat_, "types")),
      roots_key_(fh.get_key<IntsTag>(cat_, "roots")) {}

AlternativesFactory::AlternativesFactory(FileConstHandle fh)
    : cat_(fh.get_category("alternatives")),
      types_key_(fh.get_key<IntsTag>(cat_, "types")),
      roots_key_(fh.get_key<IntsTag>(cat_, "roots")) {}

NodeID AlternativesConst::get_alternative_impl(RepresentationType type,
                                               float resolution) const {
  if (!get_node().get_has_value(types_key_)) return get_node().get_id();

  double closest_resolution = get_resolution(get_node());
  int closest_index = -1;
  Nullable<Ints> types = get_node().get_value(types_key_);
  if (!types.get_is_null()) {
    Ints roots = get_node().get_value(roots_key_);
    for (unsigned int i = 0; i < types.get().size(); ++i) {
      if (types.get()[i] != type) continue;
      double cur_resolution =
          get_resolution(get_node().get_file().get_node(NodeID(roots[i])));
      if (get_resolution_metric(resolution, cur_resolution) <
          get_resolution_metric(resolution, closest_resolution)) {
        closest_index = i;
        closest_resolution = cur_resolution;
      }
    }
  }
  if (closest_index == -1) {
    return get_node().get_id();
  } else {
    return NodeID(get_node().get_value(roots_key_).get()[closest_index]);
  }
}

NodeIDs AlternativesConst::get_alternatives_impl(RepresentationType type)
    const {
  NodeIDs ret;
  if (type == PARTICLE) ret.push_back(get_node().get_id());

  if (get_node().get_has_value(roots_key_)) {
    Ints roots = get_node().get_value(roots_key_).get();
    Ints types = get_node().get_value(types_key_).get();

    for (unsigned int i = 0; i < roots.size(); ++i) {
      RMF_INTERNAL_CHECK(roots[i] != 0, "The root can't be an alternative rep");
      if (RepresentationType(types[i]) == type) ret.push_back(NodeID(roots[i]));
    }
  }
  return ret;
}

double get_resolution(NodeConstHandle root) {
  ExplicitResolutionFactory erf(root.get_file());
  if (erf.get_is(root)) {
    return erf.get(root).get_static_explicit_resolution();
  } else {
    IntermediateParticleFactory ipcf(root.get_file());
    GaussianParticleFactory gpf(root.get_file());
    std::pair<double, bool> total = get_resolution_impl(root, ipcf, gpf);
    RMF_USAGE_CHECK(total.second,
                  std::string("No particles were found at ") + root.get_name());
    return 1.0 / total.first;
  }
}

Alternatives::Alternatives(NodeHandle nh, IntsKey types_key, IntsKey roots_key)
    : AlternativesConst(nh, types_key, roots_key) {}

void Alternatives::add_alternative(NodeHandle root, RepresentationType type) {
  RMF_USAGE_CHECK(root.get_id() != NodeID(0),
                  "The root can't be an alternative");
  get_node()
      .get_shared_data()
      ->access_static_value(get_node().get_id(), types_key_)
      .push_back(type);
  get_node()
      .get_shared_data()
      ->access_static_value(get_node().get_id(), roots_key_)
      .push_back(root.get_id().get_index());

  RMF_INTERNAL_CHECK(get_alternatives(type).size() >= 1, "None found");
}

NodeConstHandle AlternativesConst::get_alternative(RepresentationType type,
                                                   double resolution) const {
  return get_node().get_file().get_node(get_alternative_impl(type, resolution));
}

NodeConstHandles AlternativesConst::get_alternatives(RepresentationType type)
    const {
  NodeConstHandles ret;
  for(NodeID nid : get_alternatives_impl(type)) {
    ret.push_back(get_node().get_file().get_node(nid));
  }
  return ret;
}

RepresentationType AlternativesConst::get_representation_type(NodeID id) const {
  if (id == get_node().get_id()) return PARTICLE;

  Ints roots = get_node().get_value(roots_key_);
  for (unsigned int i = 0; i < roots.size(); ++i) {
    if (roots[i] == static_cast<int>(id.get_index())) {
      return RepresentationType(get_node().get_value(types_key_).get()[i]);
    }
  }
  RMF_THROW(Message("No such alternative representation"), UsageException);
}

namespace {
Floats get_resolutions_impl(NodeConstHandle root, AlternativesFactory af,
                            RepresentationType type) {
  Floats ret;
  if (af.get_is(root)) {
    for(NodeConstHandle a : af.get(root).get_alternatives(type)) {
      ret.push_back(get_resolution(a));
    }
  } else {
    for(NodeConstHandle ch : root.get_children()) {
      Floats cur = get_resolutions_impl(ch, af, type);
      ret.insert(ret.end(), cur.begin(), cur.end());
    }
  }
  return ret;
}
}

Floats get_resolutions(NodeConstHandle root, RepresentationType type,
                       double accuracy) {
  AlternativesFactory af(root.get_file());
  Floats unclustered = get_resolutions_impl(root, af, type);
  if (unclustered.empty()) unclustered.push_back(1.0);
  std::sort(unclustered.begin(), unclustered.end());
  double lb = unclustered[0];
  double ub = lb;
  Floats ret;
  for(double r : unclustered) {
    if (r > lb + accuracy) {
      ret.push_back(.5 * (lb + ub));
      lb = r;
    }
    ub = r;
  }
  ret.push_back(.5 * (lb + ub));
  return ret;
}

} /* namespace decorator */
} /* namespace RMF */

RMF_DISABLE_WARNINGS
