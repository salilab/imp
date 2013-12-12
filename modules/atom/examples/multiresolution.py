## \example multiresolution.py
# This example shows how to use the multiresolution support available in
# IMP.atom via the IMP.atom.Resolution decorator and IMP.atom.Selection

import IMP.atom
import IMP.atom

# Create an IMP model and add a heavy atom-only protein from a PDB file
m = IMP.kernel.Model()

root = IMP.atom.Hierarchy.setup_particle(m.add_particle("root"))
rigid_body = IMP.core.RigidBody.setup_particle(
    m.add_particle("rigid body"),
    IMP.algebra.ReferenceFrame3D())

prot = IMP.atom.read_pdb(IMP.atom.get_example_path("example_protein.pdb"), m,
                         IMP.atom.NonWaterNonHydrogenPDBSelector())
root.add_child(prot)
rigid_body.add_member(IMP.atom.create_rigid_body(prot))

residues = IMP.atom.create_simplified_along_backbone(prot, 1)
resolutions = IMP.atom.Resolution.setup_particle(root, residues)
rigid_body.add_member(IMP.atom.create_rigid_body(residues))

triplets = IMP.atom.create_simplified_along_backbone(prot, 3)
resolutions.add_resolution(triplets)


whole = IMP.atom.create_simplified_along_backbone(prot, 10000)
resolutions.add_resolution(whole)

print IMP.atom.Selection(root, residue=15)
print IMP.atom.Selection(root, residue=15, resolution=100)
print IMP.atom.Selection(root, residue=15, resolution=10)
