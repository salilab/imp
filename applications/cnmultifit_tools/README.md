cnmultifit {#cnmultifit_bin}
==========

Fit atomic structures into cyclic symmetric EM density maps.
This follows a protocol for generating atomic structures of complexes with
cyclic symmetry (such as rings), using only the atomic structure of a single
subunit and a cryo-electron microscopy density map of the complex.
This employs the IMP::cnmultifit module.

See also [multifit](@ref impmultifit_tools) for a similar protocol
operating on non-symmetric complexes.

A \salilab{/multifit/,webserver} is also available.

_Examples_:
 - [Modeling of GroEL](../tutorial/cnmultifit_groel.html)

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
