Two different scoring files are provided:
    - protein_ligand_pose_score.lib for use when one wants to find the
    most near-native poses of a ligand from many geometry decoys of the
    same ligand
    - protein_ligand_rank_score.lib for use when screening a compound database
    against a single protein to choose putative binders.

    The rank score is used by default.

    They are both located in the IMP.atom data directory. This directory is
    available within \imp python scripts via the IMP::atom::get_data_path() function.
    From the command line, the files can be found at \c build/data/atom if \imp
    is not installed or something like \c /usr/share/imp/data/atom if it is installed.

<b>Author(s):</b> Hao Fan, Daniel Russel
_Author(s)_: Hao Fan, Daniel Russel

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - Hao Fan, Dina Schneidmann, John Irwin, G Dong, Brian Shoichet, Andrej Sali, \quote{Statistical Potential for Modeling and Ranking Protein-Ligand Interactions}, <em>submitted</em>, 2011.
*/
