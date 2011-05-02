import IMP
import IMP.core
import IMP.atom
import IMP.container

m= IMP.Model()
# create a new IMP.atom.Hierarchy for the pdb file
mp1= IMP.atom.read_pdb(IMP.core.get_example_path('example_protein.pdb'), m)
chains= IMP.atom.get_by_type(mp1, IMP.atom.CHAIN_TYPE)
rd= IMP.atom.Hierarchy(chains[0])
# Create a rigid body from the first chain
# note that rbs != chains[0] as the bounding volume for rbs needs to include all of the
# chain, but chains[0] might have a smaller sphere associated with it.
rbs=IMP.atom.create_rigid_body(chains[0])
print "all done"
