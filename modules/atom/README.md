\brief Functionality for loading, creating, manipulating and scoring atomic structures.

Molecules and collections of molecules are represented using Hierarchy particles. Whenever possible, one should prefer to use IMP::atom::Selection and related helper functions to manipulate molecules as that provides a simple and biologically relevant way of describing parts of molecules of interest.

The name "residue index" is used to refer to the index of the residue in the conventional description of the protein, as opposed to its index among the set of residues which are found in the current molecule. The same concept is known as the "residue number" in PDB files. This index is not necessarily unique within a Chain; however, the combination of the residue index and insertion code should be.

Several helper programs are also provided. They all take the name of the PDB
file as the first argument:

# pdb_check {#pdb_check_bin}
Check a PDB file for problems.

# ligand_score {#ligand_score_bin}
Score ligand poses.
To score the poses, use the `ligand_score` command line tool.

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

# Info

_Author(s)_: Daniel Russel, Ben Webb, Dina Schneidman, Javier Velazquez-Muriel, Hao Fan

_Maintainer_: `benmwebb`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See [main IMP papers list](@ref publications).
