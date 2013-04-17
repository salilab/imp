This module provides a variety of functionality for loading, creating, manipulating and scoring atomic structures. Molecules and collections of molecules are represented using Hierarchy particles. Whenever possible, one should prefer to use IMP::atom::Selection and related helper functions to manipulate molecules as that provides a simple and biologically relevant way of describing parts of molecules of interest.

The name "residue index" is used to refer to the index of the residue in the conventional description of the protein, as opposed to its index among the set of residues which are found in the current molecule. The same concept is know as the "residue number" in pdb files. This index is not necessarily unique within a Chain, however, the combination of the residue index and insertion code should be.

_Author(s)_: Daniel Russel, Ben Webb, Dina Schneidman, Javier Velazquez-Muriel

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
