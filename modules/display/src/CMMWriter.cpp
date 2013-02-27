/**
 *  \file CMMWriter.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/CMMWriter.h"
#include <IMP/base/deprecation_macros.h>
IMPDISPLAY_BEGIN_NAMESPACE
#if IMP_HAS_DEPRECATED
void CMMWriter::do_open() {
  IMP_DEPRECATED_OBJECT(IMP::rmf);
  get_stream() << "<marker_set name=\"" <<get_name() << "\">"<<std::endl;
  //std::cout << "Initializing" << std::endl;
  marker_index_=0;
}

void CMMWriter::do_close() {
  get_stream() << "</marker_set>" << std::endl;
}


bool CMMWriter::handle_sphere(SphereGeometry *g, Color color,
                        std::string name) {
  // evil, need this first to make sure that marker_index_ is
  // initialized. Otherwise marker_index_ is undefined in the
  // statement below.
  get_stream();
  IMP_INTERNAL_CHECK(marker_index_
                     < std::numeric_limits<unsigned int>::max()/10,
                     "Uninitialized marker index id");
  get_stream() << "<marker id=\"" << ++marker_index_ << "\""
               << " x=\"" << g->get_geometry().get_center()[0] << "\""
               << " y=\"" << g->get_geometry().get_center()[1] << "\""
               << " z=\"" << g->get_geometry().get_center()[2] << "\""
               << " radius=\"" << g->get_geometry().get_radius() << "\""
               << " r=\"" << color.get_red() << "\""
               << " g=\"" << color.get_green() << "\""
               << " b=\"" << color.get_blue() <<  "\""
               << " note=\"" << name <<  "\"/>" << std::endl;
  IMP_INTERNAL_CHECK(marker_index_
                     < std::numeric_limits<unsigned int>::max()/10,
                     "Uninitialized marker index id");
  return true;
}

bool CMMWriter::handle_point(PointGeometry *g, Color color,
                        std::string name) {
  IMP_INTERNAL_CHECK(marker_index_
                     < std::numeric_limits<unsigned int>::max()/10,
                     "Uninitialized marker index id");
  get_stream() << "<marker id=\"" << ++marker_index_ << "\""
               << " x=\"" << g->get_geometry().operator[](0) << "\""
               << " y=\"" << g->get_geometry().operator[](1) << "\""
               << " z=\"" << g->get_geometry().operator[](2) << "\""
               << " radius=\"" << 1 << "\""
               << " r=\"" << color.get_red() << "\""
               << " g=\"" << color.get_green() << "\""
               << " b=\"" << color.get_blue() <<  "\""
               << " note=\"" << name <<  "\"/>" << std::endl;
  return true;
}
#endif
IMPDISPLAY_END_NAMESPACE
