Resolution in PMI {#pmi_resolution}
=================

Very generally, the _representation_ of a system is defined by all the
variables that need to be determined based on input information, including
the assignment of the system components to geometric objects (e.g. points,
spheres, ellipsoids, and 3D Gaussian density functions). It is common to use
a multiscale representation, where the system is represented using several
resolution scales simultaneously. The spatial restraints will be applied
to individual resolution scales as appropriate (for example, a cross-link
may be applied at residue resolution, while an excluded volume restraint
may act on a more coarse representation of the system, such as a spherical
bead representing multiple residues).

In PMI, the _resolution_ of a representation is simply the number of residues
per spherical bead. For example, resolution 10 represents the system using
a bead for every 10 residues, while resolution 1 uses a bead for each residue.
An all-atom representation is assigned a resolution of 0.

Many PMI functions take a `resolution` parameter to specify which resolution
to act on. These functions will use the closest resolution if the exact
resolution requested is not available.

@note Internally, the IMP::atom::Representation decorator is used to keep track
of all system representations at the desired resolutions.
