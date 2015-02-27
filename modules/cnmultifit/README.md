\brief Generate cyclic atomic structures using cryo-electron microscopy data.

This module implements a protocol for generating atomic structures of complexes
with cyclic symmetry (such as rings), using only the atomic structure of a
single subunit and a cryo-electron microscopy density map of the complex.
See the IMP::multifit module for a similar protocol for handling non-symmetric
complexes.

cnmultifit: command line tool {#cnmultifit_bin}
=============================

Generally, this module is not used directly; instead, the
`cnmultifit` command line tool is used. For an example, see
[Modeling of GroEL](../tutorial/cnmultifit_groel.html).

Web server
==========

A [webserver](http://salilab.org/multifit/) that uses both this module
and the IMP::multifit module is also available.

# Info

_Author(s)_: Keren Lasker

_Maintainer_: `benmwebb`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See [main IMP papers list](@ref publications).
 - K. Lasker, M. Topf, A. Sali, H. J. Wolfson, ["Inferential optimization for simultaneous fitting of multiple components into a cryoEM map of their assembly", J Mol Biol 388, 2009](http://www.ncbi.nlm.nih.gov/pubmed/19233204)
