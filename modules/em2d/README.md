\brief Restraints using electron microscopy 2D images (class averages).

The main functionalities are:
1. Raading and writing EM images
2. Basic image processing: filtering, morphological operations, segmentation
3. Image alignment and calculation of cross correlation
4. Implements several Restraints.

The restraints vary based on how accurately the fit to the class
averages is computed, which is usually related to the evaluation
speed. Pick more accurate Restraints for higher resolution images and
models. Below is the Restraints list sorted from the fastest and most
simple to the slowest and most accurate.

- PCAFitRestraint - compares how well the principal components of the
  segmented class average fit to the principal components of the
  particles

- EM2DRestraint - implements FFT based image alignment. This Restraint
in turn uses internally the Fine2DRegistrationRestraint. Don't use
the latter one directly. The information obtained after matching a
projection with the EM image is stored in RegistrationResult.

The module goes together with the EMageFit application, that uses the
EM2DRestraint for modeling of macromolecular assemblies using class
averages. The algorithm is described in the PNAS paper below.

As a side note, the class Image has a header that is specific for EM
images, but if you don't care for that information, you can use the
class to deal with any sort of image, not only EM.

# OpenCV # {#OpenCV}
The module works with OpenCV 2.1, 2.2 and 2.3. We haven't tried with newer versions yet.

# Info

_Author(s)_: Javier Velazquez-Muriel, Daniel Russel, Dina Schneidman

_Maintainer_: `benmwebb`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See [main IMP papers list](@ref publications).
 - J. A. Velazquez-Muriel, K. Lasker, D. Russel, J. Phillips, B. M. Webb, D. Schneidman-Duhovny, A. Sali, ["Assembly of macromolecular complexes by satisfaction of spatial restraints from electron microscopy images", Proc Natl Acad Sci USA **109**(46), 18821-18826, 2012](http://www.ncbi.nlm.nih.gov/pubmed/23112201).
 - D. Schneidman-Duhovny *et al.* ["A method for integrative structure determination of protein-protein complexes", Bioinformatics **28**(24), 282-9, 2012](http://www.ncbi.nlm.nih.gov/pubmed/23093611).
