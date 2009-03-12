/**
 *  \file tunnel_traits.h    \brief traits for tunnels
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMISC_INTERNAL_TUNNEL_TRAITS_H
#define IMPMISC_INTERNAL_TUNNEL_TRAITS_H

#include "../config.h"
#include <IMP/algebra/Vector3D.h>

IMPMISC_BEGIN_INTERNAL_NAMESPACE

class TunnelTraits {
  unsigned int coordinate_;
  IMP::algebra::Vector3D center_;
  Float height_;
  Float radius_;

  /*Zone, closest point
    SLAB perpendicular point on surface
    RING perpendicular point on tunnel surface
    TUNNEL same as ring
    CHANNEL point on lip
  */

 public:
  TunnelTraits(){
    coordinate_=0;
    center_= algebra::Vector3D(0,0,0);
    height_=0;
    radius_=0;
  }
  enum Zone {INSLAB, OUTSLAB, RING, TUNNEL, CHANNEL};
  Zone get_location(const algebra::Vector3D &v) const {
    double r2= get_squared_radius(v);
    if (r2 > square(radius_)) {
      // RING or SLAB
      double r= std::sqrt(r2);
      if (get_height(v) > height_) return OUTSLAB;
      if (r - radius_ < height_-get_height(v)) {
        return RING;
      } else {
        return INSLAB;
      }
    } else if (get_height(v) < height_) {
      return TUNNEL;
    } else {
      return CHANNEL;
    }
  }
  void set_coordinate(unsigned int i) {
    IMP_check(i <3, "Coordinate out of range",
              ValueException);
    coordinate_=i;
  }
  unsigned int get_coordinate() const {
    return coordinate_;
  }
  Float get_height() const {
    return height_;
  }
  Float get_radius() const {
    return radius_;
  }
  const algebra::Vector3D& get_center() const {
    return center_;
  }
  void set_center(const algebra::Vector3D &c){
    center_=c;
  }
  void set_height(Float h) {
    IMP_check(h> 0, "Height must be positive",
              ValueException);
    height_=h;
  }
  void set_radius(Float r) {
    IMP_check(r> 0, "Radius must be positive",
              ValueException);
    radius_=r;
  }
  // signed, negative is inside
  double get_distance(const algebra::Vector3D &v) const {
    switch (get_location(v)) {
    case INSLAB:
    case OUTSLAB:
      return get_height(v) - height_;
    case RING:
    case TUNNEL:
      return radius_- std::sqrt(get_squared_radius(v));
    case CHANNEL:
      {
        algebra::Vector3D rp=get_rim_point(v);
        return (rp-v).get_magnitude();
      }
    }
    // gcc is dumb
    return -666666;
  }
  double get_squared_radius(const algebra::Vector3D &v) const {
    double sd=0;
    for (int i=1; i< 3; ++i) {
      int oc= (i+coordinate_)%3;
      sd+= square(v[oc]- center_[oc]);
    }
    return sd;
  }
  algebra::Vector3D
  get_displacement_unit_vector(const algebra::Vector3D &v) const {
    switch (get_location(v)) {
    case INSLAB:
    case OUTSLAB:
      {
      algebra::Vector3D rv(0,0,0);
      rv[coordinate_]=1;
      if (v[coordinate_] > center_[coordinate_]) {
        return rv;
      } else {
        return -rv;
      }
      }
    case RING:
    case TUNNEL:
      {
      algebra::Vector3D diff=(get_pole_point(v)-v);
      if (diff.get_squared_magnitude() < .001) {
        return -(get_rim_point(v)-v).get_unit_vector();
      } else {
        return (get_pole_point(v)-v).get_unit_vector();
      }
      }
    case CHANNEL:
      {
        algebra::Vector3D diff=(-get_rim_point(v)+v);
        if (diff.get_squared_magnitude() < .001) {
          return (get_pole_point(v)-v).get_unit_vector();
        } else {
          return diff.get_unit_vector();
        }
      }
    }
    // gcc is dumb
    return algebra::Vector3D(0,0,0);
  }
  double get_height(const algebra::Vector3D &v) const {
    return std::abs(v[coordinate_]- center_[coordinate_]);
  }
  algebra::Vector3D get_pole_point(algebra::Vector3D v) const {
    for (unsigned int i=0; i< 3; ++i) {
      if (i== coordinate_) continue;
      v[i]= center_[i];
    }
    return v;
  }
  algebra::Vector3D get_rim_point(algebra::Vector3D v) const {
    algebra::Vector3D ret=v-get_pole_point(v);
    if (ret.get_squared_magnitude() < .0001) {
      ret=algebra::Vector3D(1,1,1);
    }
    ret[coordinate_]=0;
    ret= ret.get_unit_vector()*radius_;
    if (v[coordinate_] > center_[coordinate_]) {
      ret[coordinate_]= center_[coordinate_]+height_;
    } else {
      ret[coordinate_]= center_[coordinate_]-height_;
    }
    return ret;
  }
  void show(std::ostream &out= std::cout) const {
    out << coordinate_ << " " << center_ << " " << height_ << " "
        << radius_ << std::endl;
  }
};

IMP_OUTPUT_OPERATOR(TunnelTraits);
IMPMISC_END_INTERNAL_NAMESPACE

#endif  /* IMPMISC_INTERNAL_TUNNEL_TRAITS_H */
