/** \example core/simple.cpp

    Simple example of using the IMP C++ library.

    This should be equivalent to the first part of the Python example simple.py.
*/

#include <fstream>

#include <IMP/kernel.h>
#include <IMP/algebra.h>
#include <IMP/core.h>

int main()
{
  IMP_NEW(m, IMP::Model, ());

  // Create two "untyped" Particles
  IMP_NEW(p1, IMP::Particle, (m));
  IMP_NEW(p2, IMP::Particle, (m));

  // "Decorate" the Particles with x,y,z attributes (point-like particles)
  IMP::core::XYZ d1 = IMP::core::XYZ::setup_particle(p1);
  IMP::core::XYZ d2 = IMP::core::XYZ::setup_particle(p2);

  // Use some XYZ-specific functionality (set coordinates)
  d1.set_coordinates(IMP::algebra::Vector3D(10.0, 10.0, 10.0));
  d2.set_coordinates(IMP::algebra::Vector3D(-10.0, -10.0, -10.0));
  std::cout << d1 << " " << d2 << std::endl;

  return 0;
}
