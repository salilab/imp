/**
 *  \file IMP/npcassembly/SlabWithSphericalIndent.cpp
 *  \brief Decorator for a surface with a spherical indent.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/npcassembly/SlabWithSphericalIndent.h>
#include <math.h>

IMPNPCASSEMBLY_BEGIN_NAMESPACE

        void
        SlabWithSphericalIndent::do_setup_particle(IMP::Model* m,
                                                   ParticleIndex pi,
                                                   double R,
                                                   double h)
        {
            // for now set all to non-optimizable
            m->add_attribute(get_sphere_radius_key(), pi, R, false);
            m->add_attribute(get_sphere_depth_key(), pi, h, false);

            //double alpha = sqrt();

            //m->add_attribute(get_base_circle_radius_key(), pi, alpha, false);
        }

        StringKey SlabWithSphericalIndent::get_name_key() {
            static StringKey k("SlabWithSphericalIndent");
            return k;
        }

        FloatKey SlabWithSphericalIndent::get_sphere_radius_key() {
            static FloatKey k("sphere_radius");
            return k;
        }

        FloatKey SlabWithSphericalIndent::get_sphere_depth_key() {
            static FloatKey k("sphere_depth");
            return k;
        }



        void SlabWithSphericalIndent::show(std::ostream &out) const {
            out << "SlabWithSphericalIndent"
                   << " radius="
                   << get_sphere_radius()
                    << " depth="
                       << get_sphere_depth();
        }

IMPNPCASSEMBLY_END_NAMESPACE
