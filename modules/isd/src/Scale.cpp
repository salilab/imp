/**
 *  \file Scale.cpp
 *  \brief defines a one-dimensional scale parameter (sigma, gamma, ...)
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/isd/Scale.h"

IMPISD_BEGIN_NAMESPACE

FloatKey Scale::get_scale_key() {
  static FloatKey k("scale");
  return k;
}

FloatKey Scale::get_upper_key() {
  static FloatKey k("sc_up");
  return k;
}

FloatKey Scale::get_lower_key() {
  static FloatKey k("sc_lo");
  return k;
}

void Scale::show(std::ostream &out) const {
  out << get_lower() << " < Scale " << get_scale() << " < " 
      << get_upper() <<  std::endl;
}

void Scale::set_upper(Float d) {
    Float d_;
    if (d < 0) {
        d_ = std::numeric_limits<double>::infinity();
    } else {
        d_ = d;
    }
    get_particle()->set_value(get_upper_key(), d_);
}

void Scale::set_lower(Float d) {
    Float d_ = std::max(d,0.0);
    get_particle()->set_value(get_lower_key(), d_);
}

void Scale::set_scale(Float d) {
    Float d_;
    Float lo = get_lower();
    Float up = get_upper();
    if (d < lo) {
        d_ = lo;
    } else if (d > up) {
        d_ = up;
    } else {
        d_ = d;
    }
    get_particle()->set_value(get_scale_key(), d_);
}

Scale Scale::setup_particle(Particle *p) {
    p->add_attribute(get_scale_key(), 1.0);
    p->add_attribute(get_upper_key(), std::numeric_limits<double>::infinity());
    p->add_attribute(get_lower_key(), 0.0);
    return Scale(p);
}

Scale Scale::setup_particle(Particle *p, double scale) {
    if (scale < 0) {
        IMP_THROW("Scale is positive!", ModelException);
    }
    p->add_attribute(get_scale_key(), scale);
    p->add_attribute(get_upper_key(), std::numeric_limits<double>::infinity());
    p->add_attribute(get_lower_key(), 0.0);
    return Scale(p);
}

Scale Scale::setup_particle(Particle *p, double scale, 
        double lower, double upper) {
    double lo_ = std::max(lower,0.0);
    double up_;
    if (upper < lo_) {
        up_ = std::numeric_limits<double>::infinity();
    } else {
        up_ = upper;
    }
    if (lo_ > scale || up_ < scale) {
        IMP_THROW("scale value out of bounds!", ModelException);
    }
    p->add_attribute(get_scale_key(), scale);
    p->add_attribute(get_upper_key(), up_);
    p->add_attribute(get_lower_key(), lo_);
    return Scale(p);
}


IMPISD_END_NAMESPACE
