
import IMP
import IMP.algebra as alg
import IMP.atom as atom
import IMP.core as core
import logging


def apply_random_transform(rb, max_trans= 100):
    """
        Apply a random transformation to the rigid body and change the reference
        frame
    """
    bb = alg.BoundingBox3D(alg.Vector3D(-max_trans,-max_trans,-max_trans),
                            alg.Vector3D(max_trans, max_trans, max_trans))
    Trand = alg.Transformation3D( alg.get_random_rotation_3d(),
                            alg.get_random_vector_in(bb) )
    ref = rb.get_reference_frame()
    Tr = ref.get_transformation_to()
    T = alg.compose(Trand, Tr)
    rb.set_reference_frame(alg.ReferenceFrame3D(T))
    return Trand

def apply_transformation_to_hierarchy(prot, T, fn_write=False):
    """
        If fn_write is different from False, write to file
    """
    R = T.get_rotation()
    t = T.get_translation()
    xyz1 = [core.XYZ(l) for l in atom.get_leaves(prot)]
    coords = [p.get_coordinates() for p in xyz1]
    newvs = [R.get_rotated(v) + t for v in coords]
    for i in range(len(newvs)):
        xyz1[i].set_coordinates(newvs[i])
    if(fn_write):
        atom.write_pdb(prot, fn_write)
