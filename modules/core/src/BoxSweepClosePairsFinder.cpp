/**
 *  \file BoxSweepClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/BoxSweepClosePairsFinder.h"
#include "IMP/core/XYZDecorator.h"

/* compile the CGAL code with NDEBUG since it doesn't have the
   same level of control over errors as IMP
*/
#define NDEBUG
#ifdef IMP_USE_CGAL
#include <CGAL/box_intersection_d.h>
#include <vector>
#endif


IMPCORE_BEGIN_NAMESPACE


#ifdef IMP_USE_CGAL

namespace {
struct NBLBbox
{
  XYZDecorator d_;
  typedef Float NT;
  typedef void * ID;
  Float r_;
  NBLBbox(){}
  NBLBbox(Particle *p,
                  Float r): d_(p),
                            r_(r){}
  static unsigned int dimension() {return 3;}
  void *id() const {return d_.get_particle();}
  NT min_coord(unsigned int i) const {
    return d_.get_coordinate(i)-r_;
  }
  NT max_coord(unsigned int i) const {
    return d_.get_coordinate(i)+r_;
  }
  // make it so I can reused the callback provide by NBLSS
  operator Particle*() {return d_.get_particle();}
};

static void copy_particles_to_boxes(const ParticleContainer *ps,
                                    FloatKey rk, Float distance,
                                    std::vector<NBLBbox> &boxes)
{
  boxes.resize(ps->get_number_of_particles());
  for (unsigned int i=0; i< ps->get_number_of_particles(); ++i) {
    Particle *p= ps->get_particle(i);

    Float r= distance/2.0;
    if (rk != FloatKey() && p->has_attribute(rk)) {
      r+= p->get_value(rk);
    }
    boxes[i]=NBLBbox(p, r);
  }
}

struct AddToList {
  FilteredListParticlePairContainer *out_;
  AddToList(FilteredListParticlePairContainer *out): out_(out){}
  void operator()(Particle *a, Particle *b) {
    out_->add_particle_pair(ParticlePair(a,b));
  }
};

}
#endif


BoxSweepClosePairsFinder::BoxSweepClosePairsFinder(){}

BoxSweepClosePairsFinder::~BoxSweepClosePairsFinder(){}

void BoxSweepClosePairsFinder
::add_close_pairs(ParticleContainer *ca,
                  ParticleContainer *cb,
                  FilteredListParticlePairContainer *out) {
#ifdef IMP_USE_CGAL
  std::vector<NBLBbox> boxes0, boxes1;
  copy_particles_to_boxes(ca, get_radius_key(), get_distance(), boxes0);
  copy_particles_to_boxes(cb, get_radius_key(), get_distance(), boxes1);

  CGAL::box_intersection_d( boxes0.begin(), boxes0.end(),
                            boxes1.begin(), boxes1.end(), AddToList(out));
#else
  IMP_failure( "IMP built without CGAL support.", ErrorException);
#endif
}

void BoxSweepClosePairsFinder
::add_close_pairs(ParticleContainer *c,
                  FilteredListParticlePairContainer *out) {
#ifdef IMP_USE_CGAL
  std::vector<NBLBbox> boxes;
  copy_particles_to_boxes(c, get_radius_key(), get_distance(), boxes);


  CGAL::box_self_intersection_d( boxes.begin(), boxes.end(), AddToList(out));
#else
  IMP_failure("IMP built without CGAL support.", ErrorException);
#endif
}


bool BoxSweepClosePairsFinder
::get_is_implemented() {
#ifdef IMP_USE_CGAL
  return true;
#else
  return false;
#endif
}

IMPCORE_END_NAMESPACE
