/**
 *  \file IMP/display/Colored.h
 *  \brief A decorator for a particle with a color
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_COLORED_H
#define IMPDISPLAY_COLORED_H

#include <IMP/display/display_config.h>

#include "Color.h"
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! A particle with a color.
/** Many of the geometry objects will use this color
    if it is available.
 */
class IMPDISPLAYEXPORT Colored : public Decorator {
 public:
  static Colored setup_particle(Particle *p, Color c) {
    p->add_attribute(get_color_keys()[0], c.get_red(), false);
    p->add_attribute(get_color_keys()[1], c.get_green(), false);
    p->add_attribute(get_color_keys()[2], c.get_blue(), false);
    return Colored(p);
  }

  void set_color(const Color &c) {
    get_particle()->set_value(get_color_keys()[0], c.get_red());
    get_particle()->set_value(get_color_keys()[1], c.get_green());
    get_particle()->set_value(get_color_keys()[2], c.get_blue());
  }

  Color get_color() const {
    return Color(get_particle()->get_value(get_color_keys()[0]),
                 get_particle()->get_value(get_color_keys()[1]),
                 get_particle()->get_value(get_color_keys()[2]));
  }

  static bool particle_is_instance(Particle *p) {
    IMP_USAGE_CHECK((!p->has_attribute(get_color_keys()[0]) &&
                     !p->has_attribute(get_color_keys()[1]) &&
                     !p->has_attribute(get_color_keys()[2])) ||
                        (p->has_attribute(get_color_keys()[0]) &&
                         p->has_attribute(get_color_keys()[1]) &&
                         p->has_attribute(get_color_keys()[2])),
                    "Only partially colored " << p->get_name());
    return p->has_attribute(get_color_keys()[2]);
  }

  static const FloatKeys &get_color_keys();

  IMP_DECORATOR(Colored, Decorator);
};

IMP_DECORATORS(Colored, Coloreds, Particles);

IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_COLORED_H */
