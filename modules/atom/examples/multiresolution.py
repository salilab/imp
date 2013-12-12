## \example multiresolution.py
# This example shows how to use the multiresolution support available in
# IMP.atom via the IMP.atom.Resolution decorator and IMP.atom.Selection

import IMP.atom
import IMP.atom

# Create an IMP model and add a heavy atom-only protein from a PDB file
m = IMP.kernel.Model()

root = IMP.atom.Hierarchy.setup_particle(m, m.add_particle("root"))
rigid_body = IMP.core.RigidBody.setup_particle(
    m, m.add_particle("rigid body"),
    IMP.algebra.ReferenceFrame3D())

prot = IMP.atom.read_pdb(IMP.atom.get_example_path("example_protein.pdb"), m,
                         IMP.atom.NonWaterNonHydrogenPDBSelector())
root.add_child(prot)
print "adding rigid body for atoms"
rigid_body.add_member(IMP.atom.create_rigid_body(prot))

resolutions = IMP.atom.Representation.setup_particle(root)

print "creating residue level rep"
residues = IMP.atom.create_simplified_along_backbone(prot, 1)
print "adding rigid body for residues"
rigid_body.add_member(IMP.atom.create_rigid_body(residues))
resolutions.add_representation(residues)

print "creating triplet residue level rep"
triplets = IMP.atom.create_simplified_along_backbone(prot, 3)
resolutions.add_representation(triplets)
print "adding rigid body for triplets"
rigid_body.add_member(IMP.atom.create_rigid_body(triplets))

whole = IMP.atom.create_simplified_along_backbone(prot, 10000)
resolutions.add_representation(whole)

print "resolutions are", resolutions

print IMP.atom.Selection(root, residue_index=15).get_selected_particles()
print IMP.atom.Selection(root, residue_index=15, resolution=100).get_selected_particles()
print IMP.atom.Selection(root, residue_index=15, resolution=300).get_selected_particles()
print IMP.atom.Selection(root, residue_index=15, resolution=10000).get_selected_particles()
print IMP.atom.Selection(root, residue_index=15, resolution=IMP.atom.ALL_RESOLUTIONS).get_selected_particles()
