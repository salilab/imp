## \example atom/assess_dope.py
# The script shows how to assess a protein conformation using DOPE.

from __future__ import print_function
import IMP
import IMP.atom
import IMP.container
import sys

IMP.setup_from_argv(sys.argv, "assess dope")

def create_representation():
    m = IMP.Model()
    mp0 = IMP.atom.read_pdb(IMP.atom.get_example_path(
        '1fdx.B99990001.pdb'), m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    prot = IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
    return (m, prot)


def add_dope(m, prot):
    ps = IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
    for p in ps:
        if not IMP.atom.Atom.get_is_setup(p):
            print("Huh?", p)
    dpc = IMP.container.ClosePairContainer(ps, 15.0, 0.0)
    # By default the score is evaluated on all nearby pairs of atoms,
    # even atoms that are bonded (or related by angles or dihedrals).
    # This is consistent with the behavior of the score in MODELLER.
    # If you like you can exclude pairs of atoms belonging to the same residue
    # by adding a pair filter to the container:
    # f = IMP.atom.SameResiduePairFilter()
    # dpc.add_pair_filter(f)
    IMP.atom.add_dope_score_data(prot)
    dps = IMP.atom.DopePairScore(15.0)
    d = IMP.container.PairsRestraint(dps, dpc)
    return d

print("creating representation")
(m, prot) = create_representation()

print("creating DOPE score function")
d = add_dope(m, prot)

IMP.set_check_level(IMP.USAGE)
print("DOPE SCORE ::", d.evaluate(False))
