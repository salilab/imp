import os
import numpy as np

import IMP
import IMP.algebra
import IMP.atom
import IMP.core
import IMP.test

fmt = ", ".join(["%.3f"] * 7)

class Tests(IMP.test.TestCase):
    """Test the consistency of body centric initial reference frame
    construction for rigid bodies"""

    def make_binary_complex(self, conf_num=1):
        pdb_file = self.get_input_file_name("1gp2/1gp2_AB_conf%d.pdb" % conf_num)
        m = IMP.Model()
        selector = IMP.atom.ATOMPDBSelector() & \
                   IMP.atom.NonWaterNonHydrogenPDBSelector() & \
                   IMP.atom.CAlphaPDBSelector()

        root_hier = IMP.atom.read_pdb(pdb_file, m)
        rbs = []
        for chain_hier in root_hier.get_children():
            calpha_hiers = IMP.core.get_leaves(chain_hier)

            # construct reference frame places the origin at the center of mass
            # of the rigid body, but keeps axes parallel to the lab reference
            com = IMP.atom.CenterOfMass.setup_particle(IMP.Particle(m),
                                                       calpha_hiers)
            com_coord = IMP.core.XYZ(com).get_coordinates()
            tr = IMP.algebra.Transformation3D(
                IMP.algebra.get_identity_rotation_3d(), com_coord)
            rf = IMP.algebra.ReferenceFrame3D(tr)

            rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m), rf)
            [rb.add_member(h) for h in calpha_hiers]
            rbs.append(rb)
        return m, root_hier, rbs

    def _align_to_reference(self, rbs, rbs_ref, invert=False):
        # get transformation that aligns the first chain of the given conf
        # to the first chain of the reference conf
        rf = rbs[0].get_reference_frame()
        ps = [IMP.core.XYZ(p) for p in rbs[0].get_rigid_members()]

        rf_ref = rbs_ref[0].get_reference_frame()
        ps_ref = [IMP.core.XYZ(p) for p in rbs_ref[0].get_rigid_members()]

        tr = IMP.atom.get_transformation_aligning_first_to_second(ps, ps_ref)
        if invert:
            tr = tr.get_inverse()

        # apply this transformation to both chains of the given conformation
        IMP.core.transform(rbs[0], tr)
        IMP.core.transform(rbs[1], tr)

    def _get_body_centric_reference_frame_params(self, rb):
        ps = [IMP.core.XYZ(p) for p in rb.get_rigid_members()]
        rf = IMP.core.get_initial_reference_frame(ps)
        q = rf.get_transformation_to().get_rotation().get_quaternion()
        v = rf.get_transformation_to().get_translation()
        return (*v, *q)

    def _test_conf_against_reference(self, rbs, rbs_ref):
        # get reference frame params before alignment
        params_before = [self._get_body_centric_reference_frame_params(rb)
                        for rb in rbs]

        # get reference frame params of the reference conformation
        params_ref = [self._get_body_centric_reference_frame_params(rb)
                      for rb in rbs_ref]

        # align to reference conformation
        self._align_to_reference(rbs, rbs_ref)

        # get reference frame params after alignment
        params_after = [self._get_body_centric_reference_frame_params(rb)
                        for rb in rbs]

        # restore the conformations
        self._align_to_reference(rbs, rbs_ref, invert=True)

        #print("Rigid body 1:")
        #print("%15s" % "params_before: " + fmt % params_before[0])
        #print("%15s" % "params_ref: " + fmt % params_ref[0])
        #print("%15s" % "params_after: " + fmt % params_after[0])
        #print("--------------------------------------------------")
        #print("Rigid body 2:")
        #print("%15s" % "params_before: " + fmt % params_before[1])
        #print("%15s" % "params_ref: " + fmt % params_ref[1])
        #print("%15s" % "params_after: " + fmt % params_after[1])

        # check that for rigid body 1, params are
        # very close between ref and after. For the particular conformations
        # used in this test, the parameters of reference and after alignment
        # agreed to within 10%. However, they are reasonably close as can be
        # seen from the printed log.
        test = np.allclose(params_after[0], params_ref[0], atol=0.1)
        self.assertTrue(test)

    def test_initial_reference_frame(self):
        """
        Test consistency of body centric reference initial frames between
        two conformations of the same rigid body, aligned with a common
        reference
        """
        # create complexes from all (3) conformations
        n_conf = 3
        conf = [self.make_binary_complex(i+1) for i in range(n_conf)]

        # treating each complex as reference
        for i in range(n_conf):
            rbs_ref = conf[i][-1]
            # for each conformation
            for j in range(n_conf):
                #print("\nReference conf = %d, conf = %d" % (i+1, j+1))
                rbs = conf[j][-1]
                self._test_conf_against_reference(rbs, rbs_ref)

if __name__ == '__main__':
    IMP.test.main()
