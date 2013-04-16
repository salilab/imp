FoXS is a simple command line tool for determining small angle X-ray (SAXS)
profiles.

The IMP.saxs module contains functions that, given an atomic protein structure,
can calculate its SAXS profile using the Debye formula, and then fit this
profile against the experimentally determined one. FoXS is a simple command
line interface to this functionality which takes as input a number of PDB
files and/or SAXS profiles. There is also a \salilab{foxs/,web server}
available.

_Author(s)_: Dina Schneidman

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - Dina Schneidman-Duhovny, Michael Hammel, Andrej Sali, \quote{FoXS: A Web server for Rapid Computation and Fitting of SAXS Profiles}, <em>Nucleic Acids Research</em>, 2010.
