/**
 *  \file connolly_surface.cpp     \brief Generate surface for a set of atoms
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/connolly_surface.h>
#include <IMP/algebra/connolly_surface.h>
#include <iostream>
#include <iomanip>
#include <fstream>

IMPMULTIFIT_BEGIN_NAMESPACE

void write_connolly_surface(atom::Atoms atoms, TextOutput fn,
                            float density, float probe_radius) {

  algebra::Sphere3Ds spheres;
  for (unsigned int i = 0; i < atoms.size(); ++i) {
    spheres.push_back(core::XYZR(atoms[i]).get_sphere());
  }

  algebra::ConnollySurfacePoints sps =
      algebra::get_connolly_surface(spheres, density, probe_radius);

  for (unsigned int i = 0; i < sps.size(); ++i) {
    fn.get_stream() << std::setw(5) << sps[i].get_atom(0) + 1 << std::setw(5)
                    << sps[i].get_atom(1) + 1 << std::setw(5)
                    << sps[i].get_atom(2) + 1 << std::fixed << std::setw(8)
                    << std::setprecision(3) << sps[i].get_surface_point()[0]
                    << std::setw(8) << std::setprecision(3)
                    << sps[i].get_surface_point()[1] << std::setw(8)
                    << std::setprecision(3) << sps[i].get_surface_point()[2]
                    << std::setw(8) << std::setprecision(3) << sps[i].get_area()
                    << std::setw(7) << std::setprecision(3)
                    << sps[i].get_normal()[0] << std::setw(7)
                    << std::setprecision(3) << sps[i].get_normal()[1]
                    << std::setw(7) << std::setprecision(3)
                    << sps[i].get_normal()[2] << "  0.500" << std::endl;
  }
}

IMPMULTIFIT_END_NAMESPACE
