/* Example of using the IMP C++ library */

#include <fstream>

#include <IMP.h>
#include <IMP/algebra.h>
#include <IMP/core.h>

int main()
{
  IMP::Pointer<IMP::Model> m = new IMP::Model();

  // Create two "untyped" Particles
  IMP::Pointer<IMP::Particle> p1 = new IMP::Particle(m);
  IMP::Pointer<IMP::Particle> p2 = new IMP::Particle(m);

  // "Decorate" the Particles with x,y,z attributes (point-like particles)
  IMP::core::XYZ d1 = IMP::core::XYZ::setup_particle(p1);
  IMP::core::XYZ d2 = IMP::core::XYZ::setup_particle(p2);

  // Use some XYZ-specific functionality (set coordinates)
  d1.set_coordinates(IMP::algebra::Vector3D(10.0, 10.0, 10.0));
  d2.set_coordinates(IMP::algebra::Vector3D(-10.0, -10.0, -10.0));
  std::cout << d1 << " " << d2 << std::endl;

  return 0;
}
