\brief Interface to the [Modeller](http://salilab.org/modeller/) comparative modeling package.

Functionality includes:

 - \link IMP::modeller::ModelLoader Loading a Modeller model into IMP\endlink
 - \link IMP::modeller::ModellerRestraints Calling the Modeller scoring function from IMP\endlink
 - \link IMP::modeller::IMPRestraints Calling the IMP scoring function from Modeller\endlink
 - Converting a set of Modeller \link IMP::modeller::ModelLoader.load_static_restraints() static restraints\endlink or \link IMP::modeller::ModelLoader.load_dynamic_restraints() dynamic restraints\endlink into equivalent IMP restraints

# Modeller # {#modeller}

If you want to use IMP with MODELLER, you should use version 9v7 or
later. If you installed the MODELLER Linux RPM or Mac package, it should be
detected automatically. Make sure that MODELLER is found in your `PYTHONPATH`.

# Info

_Author(s)_: Ben Webb, Daniel Russel

_Maintainer_: `benmwebb`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See [main IMP papers list](@ref publications).
