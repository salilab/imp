This module allows to use restraints coming from EM class averages.
The main class is EM2DRestraint, that you can add to any of your models. This class in turn uses internally the Fine2DRegistraitionRestraint.
Don't use the later one directly. Another interesting class is RegistrationResult,
that stores all the information obtained after matching a projection with the EM image.
And of course Image, which manages the EM images. Finally, we provide a number of
routines for image processing, like alignment, filtering or morphological operations.

The module goes together with the EMageFit application, that uses the EM2DRestraint for
modeling of macromolecular assemblies using class averages. The algorithm is described
in the PNAS paper below.

As a side note, the class Image has a header that is specific for EM images, but if you
don't care for that information, you can use the class to deal with any sort of
image, not only EM.

The module works with OpenCV 2.1, 2.2 and 2.3. We haven't tried with newer versions yet.

<b>Author(s):</b> J. Velazquez-Muriel, D. Russel

_Author(s)_: Javier Velazquez-Muriel, Daniel Russel

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - Javier A. Velazquez-Muriel, Keren Lasker, Daniel Russel, Jeremy Phillips, Benjamin M. Webb, Dina Scheidmann, Andrej Sali, \quote{Assembly of macromolecular complexes by satisfaciton of spatial restraints from electron microscopy images}, <em>Proc Natl Acad Sci USA 109(46), 18821-18826</em>, 2012.
