# Multifit Tools

MultiFit is a protocol for simultaneously fitting atomic structures
of components into the cryo-electron microscopy (cryo-EM) density map of
their assembly. It can also incorporate connectivity information from
proteomics experiments (such as cross links), and can work with density maps
at resolutions as low as 25A.

See also [cnmultifit](@ref IMP_cnmultifit_tools_overview) for a similar protocol
operating on symmetric complexes.

The component positions and orientations are optimized with respect to a
scoring function that includes the quality-of-fit of components in the map,
the protrusion of components from the map envelope, and the shape
complementarity between pairs of components. The scoring function is optimized
by an exact inference optimizer DOMINO that efficiently finds the global
minimum in a discrete sampling space.
The protocol employs the IMP::multifit module.

A [webserver](http://www.salilab.org/multifit) is also available.

_Examples_:
 - [Modeling of 3sfd](@ref multifit_3sfd)

# Info

_Author(s)_: Keren Lasker

_Maintainer_: `benmwebb`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - Keren Lasker, Maya Topf, Andrej Sali, \quote{Determining macromolecular assembly structures by molecular docking and fitting into an electron density map}, <em>Journal of Molecular Biology</em>, 2009.
 - Keren Lasker, Daniel Russel, Jeremy Phillips, Haim Wolfson, Andrej Sali, \quote{Determining architectures of macromolecular assemblies by aligning interaction networks to electon microscopy density maps}, <em>submitted</em>
