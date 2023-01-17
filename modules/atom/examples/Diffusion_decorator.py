## \example atom/Difffusion_decorator.py
# This is a simple example using the Diffusion  decorator to set the coordinates
# and translational diffusion coeffieicent of some particles

import IMP
import IMP.core
import IMP.atom
import IMP.algebra
import sys

C0=[0,1,2] # in A
C1=[1,2,3] # in A
D1 = 3.0 # in A^2/fs
R0=5 # in A
IMP.setup_from_argv(sys.argv, "Diffusion decorator example")
m = IMP.Model()

# Setup a diffusion particle with an implicitly-set diffusion coefficient
# using the Stokes-Einstein equation, treating radius as the Stokes radius
p0 = m.add_particle("diffusion0")
xyzr0 = IMP.core.XYZR.setup_particle(m, p0,
                                     IMP.algebra.Sphere3D(C0, R0))
d0 = IMP.atom.Diffusion.setup_particle(m, p0)
print("D automatically set to {:.2e} A^2/fs for radius {:.1f} A"
      .format(d0.get_diffusion_coefficient(), xyzr0.get_radius()))

# Explicitly change the diffusion coefficient:
d0.set_diffusion_coefficient(d0.get_diffusion_coefficient()/2.0)
print("D reset to {:.2e} A^2/fs".format(d0.get_diffusion_coefficient()))

# Setup a diffusion particle with an explicitly-set diffusion coefficient
p1 = m.add_particle("diffusion1")
d1 = IMP.atom.Diffusion.setup_particle(m, p1, C1, D1)
print("D explicitly set to {:.2e} A^2/fs for coordinates {} A"
      .format(d1.get_diffusion_coefficient(), d1.get_coordinates()))
