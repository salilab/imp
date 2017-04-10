/**
 *  \file RMF/validate.h
 *  \brief Support for validating the data in a file.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "RMF/FileConstHandle.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/validate.h"
#include "RMF/compiler_macros.h"
#include "RMF/log.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/shape.h"
#include "RMF/decorator/sequence.h"
RMF_ENABLE_WARNINGS
namespace RMF {
namespace {
bool validate_impl(NodeConstHandle cur, decorator::ParticleFactory pcf,
                   decorator::ResidueFactory rcf, decorator::AtomFactory acf) {
  NodeConstHandles ch = cur.get_children();
  bool ret = true;
  if (ch.empty() && cur.get_type() == REPRESENTATION) {
    if (!pcf.get_is(cur)) {
      RMF_WARN("Leaf is not a particle at " << cur.get_name());
      ret = false;
    }
  }
  RMF_FOREACH(NodeConstHandle c, ch) {
    ret = ret && validate_impl(c, pcf, rcf, acf);
  }
  return ret;
}
}  // namespace
void validate(FileConstHandle fh) {
  decorator::ParticleFactory pcf(fh);
  decorator::ResidueFactory rcf(fh);
  decorator::AtomFactory acf(fh);
  bool ok = validate_impl(fh.get_root_node(), pcf, rcf, acf);
  if (!ok) {
    RMF_THROW(Message("Invalid hierarchy"), IOException);
  }
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
