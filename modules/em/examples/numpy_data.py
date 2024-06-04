## \example em/numpy_data.py
# Accessing density map data directly using NumPy.

import IMP.em
import IMP.core
import IMP.atom
import sys

IMP.setup_from_argv(sys.argv, "numpy data")

if not IMP.IMP_KERNEL_HAS_NUMPY:
    print("IMP was not built with NumPy support.")
    sys.exit(0)

dmap = IMP.em.read_map(
    IMP.em.get_example_path("input.mrc"), IMP.em.MRCReaderWriter())

# We can get the actual voxel data as a 3D NumPy array
# This is a view of the data in `dmap`, not a copy, so is efficient
# (however, it should not be used after the `dmap` object itself is destroyed).
m = dmap.get_data()
print(m.shape)

# We can use any NumPy method to query the data
print(m.min(), m.mean(), m.max())

# Count all voxels above some threshold
print(m[m >= 0.1].size)

# We can also change the data; here we do the equivalent of
# IMP.em.get_threshold_map() in place, i.e. clear any voxel that is
# below the threshold
m[m < 0.1] = 0.0
print(m.min(), m.mean(), m.max())
