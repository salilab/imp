## \example atom/cg_pdb.py
## This example shows how to properly write out a pdb of the structure results when using coarse grained rigid bodies.

import IMP.atom

m= IMP.Model()
full=IMP.atom.read_pdb(IMP.atom.get_example_path("example_protein.pdb"), m)
chain= IMP.atom.get_by_type(full, IMP.atom.CHAIN_TYPE)[0]
print chain
# for some reason the python wrapper won't make the implicit conversion to Chain
simplified= IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(chain), 3)
IMP.atom.destroy(full)
rb= IMP.atom.create_rigid_body(simplified)
original_transform= rb.get_reference_frame().get_transformation_to()


# fake optimization, just move the rigid body
tr= IMP.algebra.get_random_rotation_3d()
rb.set_reference_frame(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(tr, IMP.algebra.Vector3D(0,0,0))))

# extract the difference
diff= rb.get_reference_frame().get_transformation_to()/original_transform
reload=IMP.atom.read_pdb(IMP.atom.get_example_path("example_protein.pdb"), m)
IMP.atom.transform(reload, diff)
name=IMP.create_temporary_file("out", ".pdb")
IMP.atom.write_pdb(reload, name)
