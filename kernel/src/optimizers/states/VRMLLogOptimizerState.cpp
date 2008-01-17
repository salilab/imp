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

namespace IMP
{

VRMLLogOptimizerState::VRMLLogOptimizerState(std::string filename,
                                             const Particles &pis) :
    pis_(pis), filename_(filename), file_number_(0), call_number_(0),
    skip_steps_(0)
{
}

void VRMLLogOptimizerState::update()
{
  if (skip_steps_ == 0 || call_number_ % skip_steps_ == 0) {
    char buf[1000];
    sprintf(buf, filename_.c_str(), file_number_);
    ++file_number_;
    write(buf);
  }
  ++call_number_;
}

void VRMLLogOptimizerState::write(std::string buf) const
{
  std::ofstream out(buf.c_str());
  if (!out) {
    IMP_WARN("Error opening VRML log file " << buf);
  } else {
    /*IMP_LOG(TERSE, "Writing " << pis_.size()
      << " particles to file " << buf << "..." << std::flush);*/
    write(pis_, radius_, r_, g_, b_, out);
    //IMP_LOG(TERSE, "done" << std::endl);
  }
}

static Float snap(Float f)
{
  if (f < 0) return 0;
  if (f > 1) return 1;
  return f;
}

void VRMLLogOptimizerState::write(const Particles &pis, FloatKey rk,
                                  FloatKey r, FloatKey g, FloatKey b,
                                  std::ostream &out)
{
  out << "#VRML V2.0 utf8\n";
  out << "Group {\n";
  out << "children [\n";

  for (unsigned int i = 0; i < pis.size(); ++i) {
    try {
      Particle *p = pis[i];
      XYZDecorator xyz = XYZDecorator::cast(p);
      float x = xyz.get_x();
      float y = xyz.get_y();
      float z = xyz.get_z();
      Float rv = -1, gv = -1, bv = -1;
      if (r != FloatKey() && b != FloatKey() && g != FloatKey()
          && p->has_attribute(r) && p->has_attribute(g)
          && p->has_attribute(b)) {
        rv = snap(p->get_value(r));
        gv = snap(p->get_value(g));
        bv = snap(p->get_value(b));
      }
      Float radius = .1;
      if (rk != FloatKey() && p->has_attribute(rk)) {
        radius = p->get_value(rk);
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
      IMP_WARN("Particle " << pis[i] << " does not have "
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

} // namespace IMP
