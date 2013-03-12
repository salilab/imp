/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/decorator_utility.h>
#include <RMF/decorators.h>
#include <RMF/NodeConstHandle.h>
#include <RMF/FileConstHandle.h>
#include <utility>
#include <limits>

RMF_ENABLE_WARNINGS

namespace RMF {

namespace {
NodeConstHandles
get_particles_by_resolution_internal(const ParticleConstFactory& f,
                                     NodeConstHandle           h,
                                     double                    resolution) {
  NodeConstHandles children = h.get_children();
  NodeConstHandles ret;
  for (unsigned int i = 0; i < children.size(); ++i) {
    NodeConstHandles cur
      = get_particles_by_resolution_internal(f, children[i],
                                             resolution);
    ret.insert(ret.end(), cur.begin(), cur.end());
  }

  if (f.get_is(h)) {
    ParticleConst p = f.get(h);
    if (p.get_radius() < resolution || ret.empty()) {
      return NodeConstHandles(1, h);
    }
  }
  return ret;
}
}

NodeConstHandles get_particles_by_resolution(NodeConstHandle h,
                                             double          resolution) {
  ParticleConstFactory f(h.get_file());
  return get_particles_by_resolution_internal(f, h, resolution);
}


} /* namespace RMF */

RMF_DISABLE_WARNINGS
