# NPC complex

These scripts demonstrate the use of [IMP](http://salilab.org/imp) in the modeling of the NPC complex using data as described in Frank Alber's 2007 NPC paper. 

The scripts work with the [IMP](http://salilab.org/imp) (version 385a178).
A default build of IMP compiled with the IMP::npc module should work, but for most effective sampling, it should
be built with [MPI](http://integrativemodeling.org/nightly/doc/html/namespaceIMP_1_1mpi.html) so that replica exchange can be used.

## List of files and directories:

- `data`		            contains all relevant data
- `NPC_papers`			    contains relevant NPC papers
- `scripts`			  
  - `test_NPC_scoring_functions.py` the main IMP script modeling for NPC

## Compiling IMP with NPC-specific module:
- Clone IMP version 385a178
- Clone this repository into imp/modules/npc/.
- Compile IMP

## Running the IMP scripts for the NPC complex:
- `cd scripts`
- `python test_NPC_scoring_functions.py & > test_NPC_scoring_functions.out` (on a single processor; prepend `mpirun -np 6` or similar if you built IMP with MPI support)

## Information

_Author(s)_: Elina Tjioe, Frank Alber

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html).
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:

- Frank Alber\*, Svetlana Dokudovskaya\*, et al, [The Molecular Architecture of the Nuclear Pore Complex](http://www.nature.com/nature/journal/v450/n7170/abs/nature06405.html), Nature 450, 695-701, 2007.

