# SAXS Merge

# SAXS Merge

A method for merging SAXS profiles using Gaussian processes.

Small-Angle X-ray Scattering (SAXS) is an experimental technique that allows
gathering structural information on biomolecules in solution. High-quality
SAXS profiles are usually obtained by manual merging of scattering profiles
from different concentrations and exposure times.

SAXS Merge is a fully automated statistical method for merging SAXS profiles
using Gaussian processes. This method requires only the buffer-subtracted
profile files in a specific order. At the heart of its formulation is
nonlinear interpolation using Gaussian Processes, which provide a statement
of the problem that accounts for correlation in the data.

A [webserver](http://salilab.org/saxsmerge) is also available.

# Info

_Author(s)_: Yannick Spill

_Maintainer_: `yannickspill`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
