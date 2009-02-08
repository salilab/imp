/**
 *  \file CMMLogOptimizerState.cpp
 *  \brief A state which writes a series of CMM files.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/CMMLogOptimizerState.h>
#include <IMP/core/XYZDecorator.h>

#include <fstream>
#include <sstream>

IMPCORE_BEGIN_NAMESPACE

CMMLogOptimizerState::CMMLogOptimizerState(std::string filename,
                                             const Particles &pis) :
    pis_(pis), filename_(filename), file_number_(0), call_number_(0),
    skip_steps_(0)
{
}

void CMMLogOptimizerState::update()
{
  if (skip_steps_ == 0 || (call_number_ % skip_steps_) == 0) {
    write_next_file();
  }
  ++call_number_;
}

void CMMLogOptimizerState::write_next_file()
{
  char buf[1000];
  sprintf(buf, filename_.c_str(), file_number_);
  ++file_number_;
  write(buf);
}


void CMMLogOptimizerState::write(std::string buf) const
{
  std::ofstream out(buf.c_str());
  if (!out) {
    IMP_WARN("Error opening CMM log file " << buf);
  } else {
    IMP_LOG(VERBOSE, "Writing " << pis_.size()
            << " particles to file " << buf << "..." << std::flush);
    std::stringstream filename;
    filename << "optimization_step_" << call_number_;
    write(pis_, filename.str() ,radius_, r_, g_, b_, name_, out);
    //IMP_LOG(TERSE, "done" << std::endl);
  }
}

static Int snap(Int f)
{
  if (f < 0) return 0;
  if (f > 255) return 255;
  return f;
}

void CMMLogOptimizerState::write(const Particles &pis,
                                 const std::string &marker_set_name,
                                 FloatKey radius_key, IntKey r_key,
                                 IntKey g_key, IntKey b_key,
                                 StringKey name_key, std::ostream &out)
{
  out << "<marker_set name=\"" <<marker_set_name << "\">"<<std::endl;
  for (unsigned int i = 0; i < pis.size(); ++i) {
    try {
      Particle *p = pis[i];
      XYZDecorator xyz = XYZDecorator::cast(p);
      float x = xyz.get_x();
      float y = xyz.get_y();
      float z = xyz.get_z();
      std::string name="";
      Float rv = 0., gv = 0., bv = 0.;
      if (r_key != IntKey() && b_key != IntKey() && g_key != IntKey()
          && p->has_attribute(r_key) && p->has_attribute(g_key)
          && p->has_attribute(b_key)) {
        rv = snap(p->get_value(r_key));
        gv = snap(p->get_value(g_key));
        bv = snap(p->get_value(b_key));
      }
      if (name_key != StringKey() && p->has_attribute(name_key)) {
        name = p->get_value(name_key);
      }
      Float radius = 5.;
      if (radius_key != FloatKey() && p->has_attribute(radius_key)) {
        radius = p->get_value(radius_key);
      }
      out << "<marker id=\"" << i << "\""
          << " x=\"" << x << "\""
          << " y=\"" << y << "\""
          << " z=\"" << z << "\""
          << " radius=\"" << radius << "\""
          << " r=\"" << (1.0*rv)/255 << "\""
          << " g=\"" << (1.0*gv)/255 << "\""
          << " b=\"" << (1.0*bv)/255 <<  "\""
          << " note=\"" << name <<  "\"/>" << std::endl;
    }
    catch (InvalidStateException &e) {
      IMP_WARN("Particle " << pis[i] << " does not have "
               << " cartesian coordinates");
    }
  }
  out << "</marker_set>" << std::endl;
}


void CMMLogOptimizerState::show(std::ostream &out) const
{
  out << "Writing CMM files " << filename_ << std::endl;
}

IMPCORE_END_NAMESPACE
