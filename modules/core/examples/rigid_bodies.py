import IMP
import IMP.core
import IMP.atom
import IMP.helper

m= IMP.Model()

mp0= IMP.atom.read_pdb(IMP.get_data_directory()\
                           +'/examples/single_protein.pdb', m)
residues= IMP.atom.get_by_type(mp0, IMP.atom.Hierarchy.RESIDUE)
rbs=IMP.core.ListSingletonContainer(residues)
IMP.helper.create_rigid_bodies(rbs, IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))

mp1= IMP.atom.read_pdb(IMP.get_data_directory()\
                           +'/examples/single_protein.pdb', m)
chains= IMP.atom.get_by_type(mp1, IMP.atom.Hierarchy.CHAIN)
rd= IMP.atom.Hierarchy(chains[0])
rbd=IMP.helper.create_rigid_body(chains[0].get_particle(),
            IMP.core.XYZs(IMP.atom.get_by_type(mp1, IMP.atom.Hierarchy.ATOM)))
print "all done"
