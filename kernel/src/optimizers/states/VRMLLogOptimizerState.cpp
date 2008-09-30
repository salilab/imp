/**
 *  \file VRMLLogOptimizerState.h
 *  \brief A state which writes a series of VRML files.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <fstream>
#include <sstream>

#include "IMP/optimizers/states/VRMLLogOptimizerState.h"
#include "IMP/decorators/XYZDecorator.h"

IMP_BEGIN_NAMESPACE

VRMLLogOptimizerState::VRMLLogOptimizerState(std::string filename,
                                             const Particles &pis) :
    filename_(filename), file_number_(0), call_number_(0),
    skip_steps_(0)
{
  set_particles(pis);
}

void VRMLLogOptimizerState::update()
{
  if (skip_steps_ == 0 || (call_number_ % skip_steps_) == 0) {
    write_next_file();
  }
  ++call_number_;
}

void VRMLLogOptimizerState::write_next_file()
{
  char buf[1000];
  sprintf(buf, filename_.c_str(), file_number_);
  ++file_number_;
  write(buf);
}

void VRMLLogOptimizerState::write(std::string buf) const
{
  std::ofstream out(buf.c_str());
  if (!out) {
    IMP_WARN("Error opening VRML log file " << buf);
  } else {
    IMP_LOG(VERBOSE, "Writing " << get_number_of_particles()
            << " particles to file " << buf << "..." << std::flush);
    write(out, get_particles());
    //IMP_LOG(TERSE, "done" << std::endl);
  }
}

IMP_LIST_IMPL(VRMLLogOptimizerState, Particle, particle, Particle*, ,);
IMP_CONTAINER_IMPL(VRMLLogOptimizerState, ParticleRefiner, particle_refiner,
                   ParticleRefinerIndex ,,,);

static Float snap(Float f)
{
  if (f < 0) return 0;
  if (f > 1) return 1;
  return f;
}

void VRMLLogOptimizerState::set_color(int c, Vector3D v) {
  colors_[c]= Vector3D(snap(v[0]),
                       snap(v[1]),
                       snap(v[2]));
}

void VRMLLogOptimizerState::write(std::ostream &out, const Particles &ps) const
{
  out << "#VRML V2.0 utf8\n";
  out << "Group {\n";
  out << "children [\n";

  for (Particles::const_iterator it = ps.begin(); it != ps.end(); ++it) {
    Particle *p = *it;
    bool wasrefined=false;
    for (ParticleRefinerConstIterator prit= particle_refiners_begin();
         prit != particle_refiners_end(); ++prit) {
      if ((*prit)->get_can_refine(p)) {
        Particles refined= (*prit)->get_refined(p);
        write(out, refined);
        (*prit)->cleanup_refined(p, refined, NULL);
        wasrefined=true;
        break;
      }
    }
    if (wasrefined) continue;
    try {
      XYZDecorator xyz = XYZDecorator::cast(p);
      float x = xyz.get_x();
      float y = xyz.get_y();
      float z = xyz.get_z();
      Float rv = -1, gv = -1, bv = -1;
      if (color_ != IntKey()
          && p->has_attribute(color_)) {
        int cv = p->get_value(color_);
        if (colors_.find(cv) != colors_.end()) {
          rv= colors_.find(cv)->second[0];
          gv= colors_.find(cv)->second[1];
          bv= colors_.find(cv)->second[2];
        }
      }
      Float radius = .1;
      if (radius_ != FloatKey() && p->has_attribute(radius_)) {
        radius = p->get_value(radius_);
        //oss << ".sphere " << x << " " << y << " " << z << " " << r << "\n";
      }

      out << "Transform {\n";
      out << "  translation " << x << " " << y << " " << z << std::endl;
      out << "  children [\n";
      out << "    Shape {\n";
      if (rv >= 0 && gv >= 0 && bv >= 0) {
        /* appearance Appearance {
           material Material {
           diffuseColor 1 0 0
           }
           }
        */
        out << "      appearance Appearance {\n";
        out << "        material Material {\n";
        out << "          diffuseColor " << rv << " " << gv;
        out << " " << bv << "\n";
        out << "        }\n";
        out << "      }\n";
      }
      out << "      geometry Sphere { radius " << radius << "}\n";
      out << "    }\n";
      out << "  ]\n";
      out << "}\n";

    } catch (InvalidStateException &e) {
      IMP_WARN("Particle " << p << " does not have "
               << " cartesian coordinates");
    }
  }
  out << "]\n";
  out << "}\n";
}

void VRMLLogOptimizerState::show(std::ostream &out) const
{
  out << "Writing VRML files " << filename_ << std::endl;
}

IMP_END_NAMESPACE
