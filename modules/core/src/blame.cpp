/**
 *  \file core/utility.cpp
 *  \brief Functions to perform simple functions on a set of particles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "IMP/core/blame.h"
#include <IMP/Restraint.h>
#include <IMP/core/RestraintsScoringFunction.h>
#include <IMP/core/XYZR.h>
#include <IMP/display/Color.h>
#include <IMP/dependency_graph.h>
#include <IMP/base/map.h>
IMPCORE_BEGIN_NAMESPACE
namespace {
typedef base::map<Particle*, Particle*> ControlledBy;
void distribute_blame(Restraint *r, const ControlledBy &cb,
                  FloatKey fk, double weight) {
  RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
  if (rs) {
    weight*= rs->get_weight();
    for (unsigned int i=0; i< rs->get_number_of_restraints(); ++i) {
      distribute_blame(rs->get_restraint(i), cb, fk, weight);
    }
  } else {
    ParticlesTemp ips= IMP::get_input_particles(r->get_inputs());
    ParticlesTemp mips;
    for (unsigned int i=0; i < ips.size(); ++i) {
      if (cb.find(ips[i]) != cb.end()) {
        Particle *p= cb.find(ips[i])->second;
        mips.push_back(p);
      }
    }
    double sf= r->get_last_score()/mips.size();
    if (sf >0) {
      IMP_LOG_TERSE( "Assigning blame of " << sf
              << " to " << mips << " for " << Showable(r) << std::endl);
      for (unsigned int i=0; i< mips.size(); ++i) {
        mips[i]->set_value(fk, mips[i]->get_value(fk)+sf);
      }
    }
  }
}
}


void assign_blame(const RestraintsTemp &rs, const ParticlesTemp &ps,
                  FloatKey attribute) {
  IMP_FUNCTION_LOG;
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (ps[i]->has_attribute(attribute)) {
      ps[i]->set_value(attribute, 0);
    } else {
      ps[i]->add_attribute(attribute, 0, false);
    }
  }
  Restraints drs;
  for (unsigned int i=0; i< rs.size(); ++i) {
    Pointer<Restraint> rd= rs[i]->create_decomposition();
    if (rd) {
      drs.push_back(rd);
    }
  }
  IMP_NEW(RestraintsScoringFunction, rsf, (drs));
  rsf->evaluate(false);
  DependencyGraph dg= get_dependency_graph(IMP::internal::get_model(rs));
  // attempt to get around boost/gcc bug and the most vexing parse
  DependencyGraphVertexIndex dgi((IMP::get_vertex_index(dg)));
  ControlledBy controlled_by;
  for (unsigned int i=0; i< ps.size(); ++i) {
    ParticlesTemp cps= get_dependent_particles(ps[i], ps, dg, dgi);
    IMP_INTERNAL_CHECK(cps.size() > 0, "No dependent particles for " << ps[i]);
    for(unsigned int j=0; j< cps.size(); ++j) {
      controlled_by[cps[j]]= ps[i];
    }
  }
  for (unsigned int i=0; i< drs.size(); ++i) {
    distribute_blame(drs[i], controlled_by, attribute, 1.0);
  }
}

display::Geometries create_blame_geometries(const RestraintsTemp &rs,
                                            const ParticlesTemp &ps,
                                            double max,
                                            std::string name) {
  IMP_FUNCTION_LOG;
  FloatKey key("blame temporary key");
  assign_blame(rs, ps, key);
  if (max==NO_MAX) {
    max=-NO_MAX;
    for (unsigned int i=0; i< ps.size(); ++i) {
      max=std::max(ps[i]->get_value(key), max);
    }
    IMP_LOG_TERSE( "Maximum blame value is " << max << std::endl);
  }
  display::Geometries ret;
  for (unsigned int i=0; i< ps.size(); ++i) {
    double colorv;
    if (max==0) {
      colorv=0;
    } else {
      colorv= display::get_linear_color_map_value(0, max,
                                                  ps[i]->get_value(key));
    }
    display::Color c= display::get_hot_color(colorv);
    IMP_NEW(XYZRGeometry, g, (ps[i]));
    if (!name.empty()) {
      g->set_name(name);
    }
    g->set_color(c);
    ret.push_back(g);
  }
  return ret;
}

IMPCORE_END_NAMESPACE
