Restrainer is an IMP module that provides convenient way of inputting molecule
definition and experimental data into \imp. This module will take experimental
data in XML format, and translate it into \imp restraints to construct models at
a variety of different resolutions or levels.

The XML input data is divided into 3 parts:
1. Representation
      - Each component in the system can have either atomic or geometric shape representation.
1. RestraintSet
      - Restraint can be added for each component, between and among components in the representation.
1. Display
      - Colors for the representation components can be defined and an output file can be generated in chimera format.

The XML schema is provided to check the validation of the input data. The visualization of the XML schema can be seen
<a href="IMP_Schema.png" target="_blank">here</a>.

To check if the XML files are valid use the \c schema.xsd which can be found as part of the restrainer data.
\code
xmllint --schema schema.xsd --noout *.xml
\endcode

The process of preparing the input data for %restrainer can be best shown using simple examples.

_Author(s)_: Elina Tjioe, Keren Lasker, Jeremy Phillips, Ben Webb

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
