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
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi, Color c) {
    m->add_attribute(get_color_keys()[0], pi, c.get_red(), false);
    m->add_attribute(get_color_keys()[1], pi, c.get_green(), false);
    m->add_attribute(get_color_keys()[2], pi, c.get_blue(), false);
  }

 public:
  void set_color(const Color &c) {
    get_model()->set_attribute(get_color_keys()[0], get_particle_index(),
                           c.get_red());
    get_model()->set_attribute(get_color_keys()[1], get_particle_index(),
                           c.get_green());
    get_model()->set_attribute(get_color_keys()[2], get_particle_index(),
                           c.get_blue());
  }

  Color get_color() const {
    return Color(get_model()->get_attribute(get_color_keys()[0],
                                        get_particle_index()),
                 get_model()->get_attribute(get_color_keys()[1],
                                        get_particle_index()),
                 get_model()->get_attribute(get_color_keys()[2],
                                        get_particle_index()));
  }

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    IMP_USAGE_CHECK((!m->get_has_attribute(get_color_keys()[0], pi) &&
                     !m->get_has_attribute(get_color_keys()[1], pi) &&
                     !m->get_has_attribute(get_color_keys()[2], pi)) ||
                        (m->get_has_attribute(get_color_keys()[0], pi) &&
                         m->get_has_attribute(get_color_keys()[1], pi) &&
                         m->get_has_attribute(get_color_keys()[2], pi)),
                    "Only partially colored " << m->get_particle_name(pi));
    return m->get_has_attribute(get_color_keys()[2], pi);
  }

  static const FloatKeys &get_color_keys();

  IMP_DECORATOR_METHODS(Colored, Decorator);
  IMP_DECORATOR_SETUP_1(Colored, Color, color);
};

IMP_DECORATORS(Colored, Coloreds, kernel::Particles);

IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_COLORED_H */
