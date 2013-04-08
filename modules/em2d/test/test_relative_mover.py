import IMP
import IMP.algebra as alg
import IMP.atom as atom
import IMP.core as core
import IMP.em2d as em2d
import IMP.test
import os
import random
import csv
import itertools
import logging
log = logging.getLogger("tests")


def is_comment(list_values, comment_char = "#"):
    if(len(list_values) == 0 or list_values[0][0] == comment_char):
        return True
    return False

def parse_relative_transform(row):
    """
        Returns an relative transform with the conventions used by IMP.
        row - A list containing a splitted line from the relative output file
    """
    euler = map(float,row[8:11])
    xyz = map(float, row[5:8])
    euler = [-euler[0], -euler[1], -euler[2]]
    R = alg.get_rotation_from_fixed_zyz(*euler)
    R = R.get_inverse()
    t = alg.Vector3D(*xyz)
    return alg.Transformation3D(R, t)


def get_relative_transforms(fn_transforms):
    f = open(fn_transforms, "r")
    reader = csv.reader(f, delimiter=" ")
    rows = [d for d in reader if(not is_comment(d))]
    f.close()
    return [parse_relative_transform(row) for row in rows]

def get_internal_transform3(Trelative, rb_rec, rb_lig):
    Trb = rb_lig.get_reference_frame().get_transformation_to()
    Trec = rb_rec.get_reference_frame().get_transformation_to()
    Tdock = alg.compose(Trelative, Trb)
    Ti = alg.compose(Trec.get_inverse(), Tdock)
    return Ti

def get_docked_reference_frames(relative_transforms, rb_lig):
    Tinitial = rb_lig.get_reference_frame().get_transformation_to()
    docked_ref_frames = []
    for Trelative in relative_transforms:
        Tdock = alg.compose(Trelative, Tinitial)
        docked_rf = alg.ReferenceFrame3D(Tdock)
        docked_ref_frames.append(docked_rf)
    return docked_ref_frames



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


class Tests(IMP.test.TestCase):
    """
        Test of the routines to use relative results
    """
    def test_relative_position_mover(self, ):
        """ Test the RelativePositionMover """
        log.info("test RelativePositionMover")
        fn_rec1 = self.get_input_file_name("1suvA_xlinked.pdb")
        fn_rec2 = self.get_input_file_name("1suvC_xlinked.pdb")
        fn_lig = self.get_input_file_name("1suvE_xlinked.pdb")
        fn_tr1  = \
        self.get_input_file_name("transforms-1suvA-1suvE_reduced.txt")
        fn_tr2  = \
        self.get_input_file_name("transforms-1suvC-1suvE_filtered.txt")
        m = IMP.Model()
        sel = atom.ATOMPDBSelector()
        h_rec1 =  atom.read_pdb(fn_rec1, m, sel)
        rb_rec1 = atom.create_rigid_body(h_rec1)
        rec1_coords = [core.XYZ(l).get_coordinates()
                                        for l in atom.get_leaves(h_rec1)]
        h_rec2 =  atom.read_pdb(fn_rec2, m, sel)
        rb_rec2 = atom.create_rigid_body(h_rec2)
        rec2_coords = [core.XYZ(l).get_coordinates()
                                        for l in atom.get_leaves(h_rec2)]
        h_ligand =  atom.read_pdb(fn_lig, m, sel)
        rb_lig = atom.create_rigid_body(h_ligand)

        Ts = get_relative_transforms(fn_tr1)
        Tis1 = []
        for i, T in enumerate(Ts):
            V =  get_internal_transform3(T, rb_rec1, rb_lig)
            Tis1.append(V)
        docked_refs1 = get_docked_reference_frames(Ts, rb_lig)

        Ts = get_relative_transforms(fn_tr2)
        Tis2 = []
        for i, T in enumerate(Ts):
            V =  get_internal_transform3(T, rb_rec2, rb_lig)
            Tis2.append(V)
        docked_refs2 = get_docked_reference_frames(Ts, rb_lig)

        mv = em2d.RelativePositionMover(rb_lig, 10, 20)
        mv.add_internal_transformations(rb_rec1, Tis1)
        mv.add_internal_transformations(rb_rec2, Tis2)

        for i in range(2):
#            prob_random = 0
            ref_before = rb_lig.get_reference_frame()
            ps = mv.propose() #_move(prob_random)
            ref_after = rb_lig.get_reference_frame()
            found = False
            current_coords = [core.XYZ(l).get_coordinates()
                                        for l in atom.get_leaves(h_ligand)]
            # check all possible reference frames where the ligand could be
            for r in itertools.chain(docked_refs1, docked_refs2):
                rb_lig.set_reference_frame(r)
                docked_coords = [core.XYZ(l) for l in atom.get_leaves(h_ligand)]
                rmsd = atom.get_rmsd(current_coords, docked_coords)
                if(rmsd < 0.5):
                    found = True
            self.assertTrue(found, msg= "the proposed move is not " \
                    "in the relative solutions")

#            os.remove(fn_proposed)

if __name__ == '__main__':
    IMP.test.main()
