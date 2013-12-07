/**
 *  \file connolly_surface.cpp     \brief Generate surface for a set of atoms
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/connolly_surface.h>
#include <IMP/algebra/connolly_surface.h>
#include <iostream>
#include <iomanip>
#include <fstream>

IMPMULTIFIT_BEGIN_NAMESPACE

void write_connolly_surface(atom::Atoms as, base::TextOutput fn, float density,
                            float probe_radius) {

  algebra::Sphere3Ds spheres;
  for (unsigned int i = 0; i < as.size(); ++i) {
    spheres.push_back(core::XYZR(as[i]).get_sphere());
  }

  algebra::ConnollySurfacePoints sps =
      algebra::get_connolly_surface(spheres, density, probe_radius);

  for (unsigned int i = 0; i < sps.size(); ++i) {
    fn.get_stream() << std::setw(5) << sps[i].atom_0 + 1 << std::setw(5)
                    << sps[i].atom_1 + 1 << std::setw(5) << sps[i].atom_2 + 1
                    << std::fixed << std::setw(8) << std::setprecision(3)
                    << sps[i].surface_point[0] << std::setw(8)
                    << std::setprecision(3) << sps[i].surface_point[1]
                    << std::setw(8) << std::setprecision(3)
                    << sps[i].surface_point[2] << std::setw(8)
                    << std::setprecision(3) << sps[i].area << std::setw(7)
                    << std::setprecision(3) << sps[i].normal[0] << std::setw(7)
                    << std::setprecision(3) << sps[i].normal[1] << std::setw(7)
                    << std::setprecision(3) << sps[i].normal[2] << "  0.500"
                    << std::endl;
  }
}

IMPMULTIFIT_END_NAMESPACE
