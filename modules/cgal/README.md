Certain functionality provided in IMP requires or benefits from using CGAL.
[CGAL](http://www.cgal.org) is a library of geometry-related
algorithms and data structures written in C++. The relevant parts of
CGAL are licensed under LGPL and QPL and commercial licenses are
available if needed. More information can be found on the
[CGAL license page](http://www.cgal.org/license.html).

The module should not be used directly by users. Instead IMP.algebra and IMP.core functionality will be improved when CGAL is available. The main purpose of this module is to allow certain CGAL code to be built with different build flags than the rest of IMP. Specifically, CGAL needs IEEE floating point specs to be followed exactly.

_Author(s)_: Daniel Russel

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
