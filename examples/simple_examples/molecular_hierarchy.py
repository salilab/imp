import IMP
import IMP.core
import IMP.modeller

m = IMP.Model()
mp0= IMP.modeller.read_pdb('single_protein.pdb', m)
# get the 16th residue
r16 = IMP.core.molecular_hierarchy_get_residue(mp0, 16)
r16.show()

atoms= IMP.core.molecular_hierarchy_get_by_type(mp0, IMP.core.MolecularHierarchyDecorator.ATOM)
print len(atoms)

#get the residues
residues= IMP.core.molecular_hierarchy_get_by_type(mp0, IMP.core.MolecularHierarchyDecorator.RESIDUE)
# set the coordinates and radius of each residue to enclose its atoms
for r in residues:
    d= IMP.core.XYZRDecorator.create(r)
    atoms= IMP.core.molecular_hierarchy_get_by_type(IMP.core.MolecularHierarchyDecorator(r),
                                                    IMP.core.MolecularHierarchyDecorator.ATOM)
    IMP.core.set_enclosing_sphere(atoms, d)

# load another copy
mp1= IMP.modeller.read_pdb('single_protein.pdb', m)

p = IMP.Particle(m)
rmp= IMP.core.MolecularHierarchyDecorator.create(p, IMP.core.MolecularHierarchyDecorator.ASSEMBLY)
rmp.add_child(mp0)
rmp.add_child(mp1)
