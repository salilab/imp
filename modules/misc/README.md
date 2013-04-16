This module contains miscellaneous functionality that is _not_ expected to be used by many IMP users. As a result, the code contained in it may be poorly tested, liable to change or experimental in nature.

In particular, other modules should _not_ depend on IMP.misc (e.g. don't create a class in another module that derives from one in IMP.misc. If you find yourself relying on such functionality, discuss moving it to a specialized module or something like IMP.core or IMP.atom.

_Author(s)_: Daniel Russel

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
