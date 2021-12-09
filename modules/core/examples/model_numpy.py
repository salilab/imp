## \example core/model_numpy.py
# Demonstration using NumPy to manipulate IMP data.
#

import IMP.example
import sys

IMP.setup_from_argv(sys.argv, "model numpy")

(m, c) = IMP.example.create_model_and_particles()

if not IMP.IMP_KERNEL_HAS_NUMPY:
    print("IMP was not built with NumPy support.")
    sys.exit(0)

# Get all pairs of particles that are close to each other
nbl = IMP.container.ClosePairContainer(c, 0, 2)
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
