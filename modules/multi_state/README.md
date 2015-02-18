Functionality for handling multi-state models. These are used in IMP to
model systems that are found simultaneously in multiple states. For example,
a hinge protein may be found in both 'open' and 'closed' forms, and so some
experiments conducted on a sample of the protein will return data from both
forms. Building a single model that tries to be consistent with all the data
will therefore fail - two models need to be constructed.

# multi_foxs {#multi_foxs_bin}
Command line tool to enumerate multi-state models with SAXS profiles.
See also IMP::foxs for the similar FoXS command line tool.

# Info

_Author(s)_: Dina Schneidman

_Maintainer_: `duhovka`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - Dina Schneidman-Duhovny, Michael Hammel, John A. Tainer, Andrej Sali, \quote{Accurate SAXS profile computation and its assessment by contrast variation experiments}, <em> Biophysical Journal </em>, 2013.
