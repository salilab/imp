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
 public:
  TunnelTraits(){
    coordinate_=0;
    center_= algebra::Vector3D(0,0,0);
    height_=0;
    radius_=0;
  }
  void set_coordinate(unsigned int i) {
    IMP_check(coordinate_ <3, "Coordinate out of range",
              ValueException);
    coordinate_=i;
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
    double hd= std::abs(get_height(v))-height_;
    double ret=0;
    double r2=get_squared_distance_to_center(v);
    if (hd > 0) {
      if (r2 < square(radius_)) {
        ret= std::sqrt(square(radius_-std::sqrt(r2))+ hd*hd);
      } else {
        ret= hd;
      }
    } else {
      Float rd= radius_-std::sqrt(r2);
      if (rd > 0) return rd;
      if (rd < hd) {
        // closer to slab
        return hd;
      } else {
        return rd;
      }
    }
    //IMP_LOG(VERBOSE, "Distance of " << v << " is " << ret << std::endl);
    return ret;
  }
  /*algebra::Vector3D get_closest_point(const algebra::Vector3D &v) const {
    double h= get_height(v);
    double r= std::sqrt(get_squared_distance_to_center(v));
    algebra::Vector3D ret=v;
    if (std::abs(std::abs(h)-height_) < std::abs(r - radius_)) {
      // close to slab
      if (h >0) {
        ret[coordinate_]= height_;
      } else {
        ret[coordinate_]= -height_;
      }
    } else {
      // close to cylinder
      algebra::Vector3D diff= (v- get_pole_point(v)).get_unit_vector();
      return get_pole_point(v)+diff*radius_;
    }
    }*/
  double get_squared_distance_to_center(const algebra::Vector3D &v) const {
    double sd=0;
    for (int i=1; i< 3; ++i) {
      int oc= (i+coordinate_)%3;
      sd+= square(v[oc]- center_[oc]);
    }
    return sd;
  }
  algebra::Vector3D
  get_displacement_unit_vector(const algebra::Vector3D &v) const {
    double h= get_height(v);
    double r2= get_squared_distance_to_center(v);
    IMP_LOG(VERBOSE, "Distances are " << h*h
            << " and " << r2 << std::endl);
    if (std::abs(h)>height_) {
      // close to slab
      algebra::Vector3D diff(0,0,0);
      if (h > 0) {
        diff[coordinate_]= 1;
      } else {
        diff[coordinate_]=-1;
      }
      if (r2 < square(radius_)) {
        algebra::Vector3D pv=(get_pole_point(v)-v).get_unit_vector();
        pv= pv*std::abs(std::sqrt(r2)-radius_);
        diff= diff.get_unit_vector()*(std::abs(h)-height_)+pv;
        diff= diff.get_unit_vector();
      } else {

      }
      IMP_LOG(VERBOSE, "Displacement in slab is " << diff << std::endl);
      {
        algebra::Vector3D vb= v- diff*get_distance(v);
        IMP_assert(std::abs(get_distance(vb)) < .01,
                   "Stepping back should be close to surface: "
                   << "stepping from " << v
                   << " along " << diff
                   << " of distance " << get_distance(v)
                   << " gives distance " << get_distance(vb));
      }
      return diff;
    } else {
      // close to cylinder
      algebra::Vector3D diff= (get_pole_point(v)-v);
      if (diff.get_squared_magnitude() < .00001){
        diff=algebra::Vector3D(0,0,0);
        diff[(coordinate_+1)%3]=1;
        IMP_LOG(TERSE, "Particle is close to center" << diff << std::endl);
      } else {
        diff = diff.get_unit_vector();
      }
      IMP_LOG(VERBOSE, "Displacement in cylinder is " << diff << std::endl);
      return diff;
    }
  }
  double get_height(const algebra::Vector3D &v) const {
    return v[coordinate_]- center_[coordinate_];
  }
  algebra::Vector3D get_pole_point(algebra::Vector3D v) const {
    for (unsigned int i=0; i< 3; ++i) {
      if (i== coordinate_) continue;
      v[i]= center_[i];
    }
    return v;
  }
  void show(std::ostream &out= std::cout) const {
    out << coordinate_ << " " << center_ << " " << height_ << " "
        << radius_ << std::endl;
  }
};

IMP_OUTPUT_OPERATOR(TunnelTraits);
IMPMISC_END_INTERNAL_NAMESPACE

#endif  /* IMPMISC_INTERNAL_TUNNEL_TRAITS_H */
