## \example atom/load_protein_restrain_bonds.py
# Load a protein from a PDB file and then restrain all the bonds to have their
# current length.
#

from __future__ import print_function
import IMP.atom
import IMP.container
import sys

IMP.setup_from_argv(sys.argv, "load protein restrain bonds")

m = IMP.Model()
prot = IMP.atom.read_pdb(IMP.atom.get_example_path("example_protein.pdb"), m)
IMP.atom.add_bonds(prot)
bds = IMP.atom.get_internal_bonds(prot)
bl = IMP.container.ListSingletonContainer(m, bds)
h = IMP.core.Harmonic(0, 1)
bs = IMP.atom.BondSingletonScore(h)
br = IMP.container.SingletonsRestraint(bs, bl)
print(br.evaluate(False))
