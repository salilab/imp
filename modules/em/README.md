This module provides basic utilities for handling 2D and 3D density maps. The main functionalities are:
1. Reading and writing various density map formats such as XPLOR, MRC, EM and SPIDER.
1. Simulating density maps of particles, supports particles of any radii and mass.
1. Calculating cross-correlation scores bewteen a density map and a set of particles.
1. Implements several Restraints.

The restraints vary based on how accurately the fit to the density is scored, which is usually related to the evaluation speed. Pick more accurate Restraints for higher resolution maps and models. Below is the Restraints list sorted from the fastest and most simple to the slowest and most accurate.

- PCAFitRestraint - compares how well the principal components of the density map fit to the principal components of the particles
- DensityFillingRestraint - estimates the percentage of volume of the density map that is covered by particles
- EnvelopePenetrationRestraint - estimates the number of particles that fall outside the density map
- EnvelopeFitRestraint - scores how well the particles fit the density map using MapDistanceTransform that transforms a density map into a Distance Transform of the map envelope
- FitRestraint - computes the fit using cross correlation

_Author(s)_: Keren Lasker, Javier Velazquez-Muriel, Friedrich Foerster, Daniel Russel, Dina Schneidman

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
