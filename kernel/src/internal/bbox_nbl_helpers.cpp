/**
 *  \file bbox_nbl_helpers.cpp
 *  \brief Helpers for the CGAL-based NBL.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/internal/bbox_nbl_helpers.h"
#include "IMP/decorators/XYZDecorator.h"
#include "IMP/score_states/NonbondedListScoreState.h"

/* compile the CGAL code with NDEBUG since it doesn't have the
   same level of control over errors as IMP
*/
#define NDEBUG
#ifdef IMP_USE_CGAL
#include <CGAL/box_intersection_d.h>
#include <vector>
#endif

namespace IMP
{

namespace internal
{

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

#ifdef IMP_USE_CGAL
static void copy_particles_to_boxes(const Particles &ps,
                                    FloatKey rk, Float slack, Float cutoff,
                                    std::vector<internal::NBLBbox> &boxes)
{
  boxes.resize(ps.size());
  for (unsigned int i=0; i< ps.size(); ++i) {
    Particle *p= ps[i];

    Float r= slack+cutoff/2.0;
    if (rk != FloatKey() && p->has_attribute(rk)) {
      r+= p->get_value(rk);
    }
    boxes[i]=internal::NBLBbox(p, r);
  }
}
#endif

void bipartite_bbox_scan(const Particles &ps0, const Particles &ps1,
                         FloatKey rk, Float slack, Float cutoff,
                         const NBLAddPairIfNonbonded &ap)
{
#ifdef IMP_USE_CGAL
  std::vector<internal::NBLBbox> boxes0, boxes1;
  copy_particles_to_boxes(ps0, rk, slack, cutoff, boxes0);
  copy_particles_to_boxes(ps1, rk, slack, cutoff, boxes1);

  CGAL::box_intersection_d( boxes0.begin(), boxes0.end(),
                            boxes1.begin(), boxes1.end(), ap);
#else
  IMP_failure( "IMP built without CGAL support.", ErrorException);
#endif
}

void bbox_scan(const Particles &ps,
               FloatKey rk, Float slack, Float cutoff,
               const NBLAddPairIfNonbonded &ap)
{
#ifdef IMP_USE_CGAL
  std::vector<internal::NBLBbox> boxes;
  copy_particles_to_boxes(ps, rk, slack, cutoff, boxes);


  CGAL::box_self_intersection_d( boxes.begin(), boxes.end(), ap);
#else
  IMP_failure("IMP built without CGAL support.", ErrorException);
#endif
}

} // namespace internal

} // namespace IMP
