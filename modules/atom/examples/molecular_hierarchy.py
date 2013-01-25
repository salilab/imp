## \example atom/molecular_hierarchy.py
## In this example, we read a protein from a PDB file and set the center and radius of each residue to enclose the atoms in that residue.
##
## Then a second copy of the protein is loaded and they are both added to the same hierarchy
## to define a hypothetical assembly.
##

import IMP
import IMP.core
import IMP.atom

m = IMP.Model()
mp0= IMP.atom.read_pdb(IMP.atom.get_example_path('example_protein.pdb'), m)
# get the 16th residue of the first chain
hchain= IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
# decorate the chain particle with an IMP.atom.Chain decorator.
# unfortunately, our python wrapper does not handle converseions properly
# as a result you have to manually get the particle for that chain
chain=IMP.atom.Chain(hchain.get_particle())
r16 = IMP.atom.get_residue(chain, 16)
r16.show()

# get all the atoms
atoms= IMP.atom.get_by_type(mp0, IMP.atom.ATOM_TYPE)
# I didn't really have anything interesting to do with them...

# create a new version of the protein that is coarsened (one particle per residue)
smp0= IMP.atom.create_simplified_along_backbone(chain, 1)

# we don't need mp0 any more
IMP.atom.destroy(mp0)

# load another copy
mp1= IMP.atom.read_pdb(IMP.atom.get_example_path('example_protein.pdb'), m)

# create a graph from the hierarchy
mp1t= IMP.atom.get_hierarchy_tree(mp1)
# process the file with dot like
# dot -Tpdf hierarchy.dot > hierarchy.pdf
mp1t.show_graphviz(open("hierarchy.dot", "w"))
# try to display it graphically, assuming altgraph is installed
try:
    mp1t.show_with_altgraph()
except:
    pass

# make this one rigid
IMP.atom.create_rigid_body(mp1)

# create a hierarchy which contains the two proteins
p = IMP.Particle(m)
rmp= IMP.atom.Hierarchy.setup_particle(p)
rmp.add_child(smp0)
rmp.add_child(mp1)
