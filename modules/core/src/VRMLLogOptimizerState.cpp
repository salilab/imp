/**
 *  \file VRMLLogOptimizerState.cpp
 *  \brief A state which writes a series of VRML files.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/VRMLLogOptimizerState.h>
#include <IMP/core/XYZRDecorator.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/deprecation.h>
#include <fstream>
#include <sstream>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

VRMLLogOptimizerState::VRMLLogOptimizerState(SingletonContainer *pc,
                                             std::string filename) :
    filename_(filename), file_number_(0), call_number_(0),
    skip_steps_(0), radius_(XYZRDecorator::get_default_radius_key()),
    pc_(pc)
{
  IMP_DEPRECATED(VRMLLogOptimizerState, display::LogOptimizerState);
}

template <class It>
void VRMLLogOptimizerState::write(std::ostream &out, It b, It e) const
{
  out << "#VRML V2.0 utf8\n";
  out << "Group {\n";
  out << "children [\n";

  for (It c=b; c != e; ++c) {
    Particle *p = *c;
    bool wasrefined=false;
    for (RefinerConstIterator prit= particle_refiners_begin();
         prit != particle_refiners_end(); ++prit) {
      if ((*prit)->get_can_refine(p)) {
        Particles refined= (*prit)->get_refined(p);
        write(out, refined.begin(), refined.end());
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
    IMP_LOG(VERBOSE, "Writing " << pc_->get_number_of_particles()
            << " particles to file " << buf << "..." << std::flush);
    write(out, pc_->particles_begin(), pc_->particles_end());
    //IMP_LOG(TERSE, "done" << std::endl);
  }
}

IMP_LIST_IMPL(VRMLLogOptimizerState, Refiner, particle_refiner,
              Refiner*,,,);

static Float snap(Float f)
{
  if (f < 0) return 0;
  if (f > 1) return 1;
  return f;
}

void VRMLLogOptimizerState::set_color(int c, algebra::Vector3D v) {
  colors_[c]= algebra::Vector3D(snap(v[0]), snap(v[1]), snap(v[2]));
}


void VRMLLogOptimizerState::show(std::ostream &out) const
{
  out << "Writing VRML files " << filename_ << std::endl;
}

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE
