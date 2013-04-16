MultiFit is a computational method for simultaneously fitting atomic
structures of components into their assembly density map at
resolutions as low as 25A. The component positions and orientations
are optimized with respect to a scoring function that includes the
quality-of-fit of components in the map, the protrusion of components
from the map envelope, as well as the shape complementarity between
pairs of components. The scoring function is optimized by an exact
inference optimizer DOMINO that efficiently finds the global minimum
in a discrete sampling space. For more information ple

See the IMP::cnmultifit module for a similar protocol for handling symmetric
complexes.

Generally, this module is not used directly; instead, the
\ref application_multifit_tools "multifit.py application" is used.

For more information please see the
[Multifit](http://www.salilab.org/multifit/) website.

_Author(s)_: Keren Lasker

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - Keren Lasker, Maya Topf, Andrej Sali, Haim J. Wolfson, \quote{Inferential optimization for simultaneous fitting of multiple components into a cryoEM map of their assembly}, <em>Journal of Molecular Biology</em>, 2009.
