## \example atom/edit_molecular_hierarchy.py
## In this example, we read a protein from a PDB file and then add a layer of atom::Fragments below the chain.
##

import IMP.kernel
import IMP.core
import IMP.atom

m = IMP.kernel.Model()
ep = IMP.atom.read_pdb(IMP.atom.get_example_path('example_protein.pdb'), m)

hchain= IMP.atom.get_by_type(ep, IMP.atom.CHAIN_TYPE)[0]

children = hchain.get_children()

# create two fragments with 10 residues each and transfer the residues to be their children

f0 = IMP.atom.Fragment.setup_particle(m, m.add_particle("F0"),
                                      [IMP.atom.Residue(x).get_index() for x in children[:10]])
for c in children[:10]:
    hchain.remove_child(c)
    f0.add_child(c)

f1 = IMP.atom.Fragment.setup_particle(m, m.add_particle("F1"),
                                      [IMP.atom.Residue(x).get_index() for x in children[10:20]])
for c in children[10:20]:
    hchain.remove_child(c)
    f1.add_child(c)

# remove the rest to make the graph simple
for c in children[20:]:
    hchain.remove_child(c)

# add the fragments under the chain
hchain.add_child(f0)
hchain.add_child(f1)

# create a graph from the hierarchy
ept= IMP.atom.get_hierarchy_tree(ep)

IMP.base.show_graphviz(ept)
