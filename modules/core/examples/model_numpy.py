## \example core/model_numpy.py
# Demonstration using NumPy to manipulate IMP data.
#

import IMP.container
import sys

IMP.setup_from_argv(sys.argv, "model numpy")

if not IMP.IMP_KERNEL_HAS_NUMPY:
    print("IMP was not built with NumPy support.")
    sys.exit(0)

# Make 100 xyz particles randomly distributed in a cubic box
m = IMP.Model()
sc = IMP.container.ListSingletonContainer(m)
b = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10,10,10))
for i in range(100):
    p = m.add_particle("p")
    sc.add(p)
    d = IMP.core.XYZR.setup_particle(m, p,
                  IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(b), 1))
    d.set_coordinates_are_optimized(True)

# Get all pairs of particles that are close to each other
nbl = IMP.container.ClosePairContainer(sc, 0, 2)
m.update()

# contents is a NumPy 2xN array of the indices of each close particle pair
contents = nbl.get_contents()

# Get direct access to the coordinates and radii of all particles in the Model.
# These are NumPy views, not copies; they share memory with the Model. This
# allows us very efficient read-write access to the data, with a caveat: if
# the Model memory is reallocated (typically when particles or attributes are
# added) any views become invalid, so we should not keep them around long-term.
# These arrays are indexed by particle indices.
xyz, r = m.get_spheres_numpy()

import numpy

# Once we have coordinates and particle indices, we can use any NumPy
# function on them. Here we calculate the Cartesian distance between each
# pair of particles in the close pair container.
dists = numpy.linalg.norm(numpy.diff(xyz[contents], axis=1), axis=2).flatten()

print("Distribution of center-center particle distances:")
print("Min %.2f Max %.2f Mean %.2f" % (dists.min(), dists.max(), dists.mean()))
