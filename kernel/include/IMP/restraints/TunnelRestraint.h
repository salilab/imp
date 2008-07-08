/**
 *  \file TunnelRestraint.h    \brief Tunnel restraint.
 *
 *  Just return a constant.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_TUNNEL_RESTRAINT_H
#define __IMP_TUNNEL_RESTRAINT_H

#include "../IMP_config.h"
#include "../Restraint.h"
#include "../Vector3D.h"
#include "../UnaryFunction.h"
#include "../internal/kernel_version_info.h"

namespace IMP
{

class PairScore;

//! Restrain particles to a tunnel.
/** Particles with x,y,z coordinates and an optional radius are
    prevented from being in a volume destribed by a slab from
    (center[i]-height) to (center+height) on the ith coordinate with a
    tunnel of radius r centered at center. To set which coordinate
    is used use the get/set _coordinate functions.

    Note that the UnaryFunction should look like a lower bound with 0
    meaning that the particle is just touching the tunnel.

    \ingroup restraint
 */
class IMPDLLEXPORT TunnelRestraint : public Restraint
{
  int coordinate_;
  Vector3D center_;
  Float height_;
  Float radius_;
  Pointer<UnaryFunction> f_;
  FloatKey rk_;
public:
  TunnelRestraint(UnaryFunction* f, FloatKey rk);

  void set_center(Vector3D c){
    center_=c;
  }
  void set_height(Float h){
    IMP_check(h >= 0,
              "Height can't be negative",
              ValueException);
    height_=h;
  }
  void set_radius(Float h){
    IMP_check(h >= 0,
              "Radius can't be negative",
              ValueException);
    radius_=h;
  }
  void set_coordinate(unsigned int i) {
    IMP_check(i < 3,
              "Invalid coordinate value",
              ValueException);
    coordinate_=i;
  }
  unsigned int get_coordinate() const {
    return coordinate_;
  }

  using Restraint::add_particles;
  using Restraint::add_particle;
  using Restraint::set_particles;
  using Restraint::clear_particles;
  using Restraint::erase_particle;

  IMP_RESTRAINT(internal::kernel_version_info)
};

} // namespace IMP

#endif  /* __IMP_TUNNEL_RESTRAINT_H */
