## \example atom/score_protein_with_ligand.py
# Show how to score a number of ligand conformations loaded from a file
# against a protein loaded from a pdb.

from __future__ import print_function
import IMP.atom
import sys

IMP.setup_from_argv(sys.argv, "score protein with ligand")

m = IMP.Model()
IMP.set_check_level(IMP.NONE)
protein = IMP.atom.read_pdb(IMP.atom.get_example_path('1d3d-protein.pdb'), m)
protein_atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)
ligands = IMP.atom.read_mol2(IMP.atom.get_example_path('1d3d-ligands.mol2'), m)
# create the score which applies to a pair of atoms
ps = IMP.atom.ProteinLigandAtomPairScore()
ps.set_was_used(True)
# label each atom of the protein with the type needed for scoring
IMP.atom.add_protein_ligand_score_data(protein)
for cl in ligands.get_children():
    # compute the atom type for each ligand atom
    IMP.atom.add_protein_ligand_score_data(cl)
    score = 0
    ligand_atoms = IMP.atom.get_by_type(cl, IMP.atom.ATOM_TYPE)
    for pa in protein_atoms:
        for la in ligand_atoms:
            # check if the atoms are close enough together
            if IMP.core.get_distance(IMP.core.XYZ(pa), IMP.core.XYZ(la)) < 15:
                # score one pair of atoms
                score += ps.evaluate_index(m,
                                           (pa.get_particle_index(),
                                            la.get_particle_index()), None)
    print("score for ", cl.get_name(), "is", score)
