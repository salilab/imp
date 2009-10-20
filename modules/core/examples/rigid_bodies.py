import IMP
import IMP.core
import IMP.atom
import IMP.helper

m= IMP.Model()

mp0= IMP.atom.read_pdb(IMP.get_data_directory()\
                           +'/atom/example_protein.pdb', m)
residues= IMP.atom.get_by_type(mp0, IMP.atom.RESIDUE_TYPE)
rbs=IMP.core.ListSingletonContainer(residues)
rbss=IMP.helper.create_rigid_bodies(rbs, IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
m.add_score_state(rbss)

mp1= IMP.atom.read_pdb(IMP.get_data_directory()\
                           +'/atom/example_protein.pdb', m)
chains= IMP.atom.get_by_type(mp1, IMP.atom.CHAIN_TYPE)
rd= IMP.atom.Hierarchy(chains[0])
rbs=IMP.helper.create_rigid_body(chains[0].get_particle(),
            IMP.core.XYZs(IMP.atom.get_by_type(mp1, IMP.atom.ATOM_TYPE)))
m.add_score_state(rbs)
print "all done"
