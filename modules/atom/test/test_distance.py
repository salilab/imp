import IMP
import IMP.test
import IMP.core as core
import IMP.atom as atom
import IMP.algebra as alg
import math


class Tests(IMP.test.TestCase):

    def test_placement_score(self):
        """Test placement score"""
        m = IMP.Model()
        # read PDB
        with self.open_input_file("mini.pdb") as fh:
            mp = atom.read_pdb(fh, m, atom.NonWaterPDBSelector())
        with self.open_input_file("mini.pdb") as fh:
            mp1 = atom.read_pdb(fh, m, atom.NonWaterPDBSelector())
        xyz = core.XYZs(atom.get_leaves(mp))
        xyz1 = core.XYZs(atom.get_leaves(mp1))
        # create a random transformation
        t = IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                         IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        for d in xyz1:
            core.transform(d, t)
        da = atom.get_placement_score(xyz1, xyz)
        d = t.get_translation().get_magnitude()
        a = IMP.algebra.get_axis_and_angle(t.get_rotation()).second
        self.assertAlmostEqual(da[0], d, 2)
        self.assertAlmostEqual(da[1], a, 2)

    def test_pairwise_rmsd_score(self):
        """Test pairwise RMSD score"""
        m = IMP.Model()
        # Get reference orientation ref1, ref2
        mpref = atom.read_pdb(self.get_input_file_name("mini.pdb"),
                              m, atom.NonWaterPDBSelector())
        leaves = atom.get_leaves(mpref)
        ref1 = core.XYZs(leaves[:30])
        ref2 = core.XYZs(leaves[30:])
        # Transform the complex
        mpmdl = atom.read_pdb(self.get_input_file_name("mini.pdb"),
                              m, atom.NonWaterPDBSelector())
        leaves = atom.get_leaves(mpmdl)
        mdl1 = core.XYZs(leaves[:30])
        mdl2 = core.XYZs(leaves[30:])
        t = IMP.algebra.Transformation3D(
                IMP.algebra.get_random_rotation_3d(),
                IMP.algebra.get_random_vector_in(
                    IMP.algebra.get_unit_bounding_box_3d()))
        for d in mdl1 + mdl2:
            core.transform(d, t)
        # Both components were transformed together, so RMSD ~= 0
        self.assertAlmostEqual(
            IMP.atom.get_pairwise_rmsd_score(ref1, ref2, mdl1, mdl2),
            0.0, delta=1e-6)
        # Transform second component
        t2 = IMP.algebra.Transformation3D(
                IMP.algebra.get_identity_rotation_3d(),
                IMP.algebra.Vector3D(5.0, 0.0, 0.0))
        for d in mdl2:
            core.transform(d, t2)
        self.assertAlmostEqual(
            IMP.atom.get_pairwise_rmsd_score(ref1, ref2, mdl1, mdl2),
            5.0, delta=1e-6)

    def test_drms(self):
        """ Test drms measure """
        m = IMP.Model()
        sel = atom.CAlphaPDBSelector()
        with self.open_input_file("mini.pdb") as fh:
            prot1 = atom.read_pdb(fh, m, sel)
        with self.open_input_file("mini.pdb") as fh:
            prot2 = atom.read_pdb(fh, m, sel)
        xyzs1 = core.XYZs(atom.get_leaves(prot1))
        xyzs2 = core.XYZs(atom.get_leaves(prot2))
        drms = atom.get_drms(xyzs1, xyzs2)
        # Molecule with itself
        self.assertAlmostEqual(drms, 0)
        R = IMP.algebra.get_random_rotation_3d()
        v = IMP.algebra.get_random_vector_in(
            IMP.algebra.get_unit_bounding_box_3d())
        T = IMP.algebra.Transformation3D(R, v)
        for x in xyzs2:
            core.transform(x, T)
        drms = atom.get_drms(xyzs1, xyzs2)
        # Same thing after transformation
        self.assertAlmostEqual(drms, 0)
        #
        for x in xyzs2:
            R = IMP.algebra.get_random_rotation_3d()
            T = IMP.algebra.Transformation3D(R, v)
            core.transform(x, T)
        drms = atom.get_drms(xyzs1, xyzs2)
        self.assertTrue(drms > 0)

    def test__rigid_bodies_drms(self):
        """ Test drms measure taking into account rigid bodies"""
        m = IMP.Model()
        sel = atom.CAlphaPDBSelector()
        with self.open_input_file("mini.pdb") as fh:
            prot1 = atom.read_pdb(fh, m, sel)
        with self.open_input_file("mini.pdb") as fh:
            prot2 = atom.read_pdb(fh, m, sel)

        hchains1 = atom.get_by_type(prot1, atom.CHAIN_TYPE)
        hchains2 = atom.get_by_type(prot2, atom.CHAIN_TYPE)
        xyzs1 = core.XYZs(atom.get_leaves(prot1))
        xyzs2 = core.XYZs(atom.get_leaves(prot2))
        x = 0
        ranges = []
        for h in hchains1:
            ls1 = (atom.get_leaves(h))
            y = x + len(ls1)
            ranges.append((x, y))
            x = y
        drms = atom.get_drms(xyzs1, xyzs2)
        rb_drms = atom.get_rigid_bodies_drms(xyzs1, xyzs2, ranges)
        self.assertAlmostEqual(rb_drms, 0)
        self.assertAlmostEqual(
            drms,
            rb_drms,
            delta=1e-3,
            msg="rb_drms != drms")
        # Same thing after transformation of each of the chains
        for h in hchains2:
            R = alg.get_random_rotation_3d()
            v = alg.get_random_vector_in(alg.get_unit_bounding_box_3d())
            T = alg.Transformation3D(R, v)
            ls = atom.get_leaves(h)
            for l in ls:
                core.transform(IMP.core.XYZ(l), T)
        drms = atom.get_drms(xyzs1, xyzs2)
        rb_drms = atom.get_rigid_bodies_drms(xyzs1, xyzs2, ranges)
        self.assertAlmostEqual(drms, rb_drms, delta=0.3, msg="rb_drms != drms")

    def test__rigid_bodies_drmsd(self):
        """ Test drmsd measure"""
        m = IMP.Model()
        sel = atom.CAlphaPDBSelector()
        with self.open_input_file("mini.pdb") as fh:
            prot1 = atom.read_pdb(fh, m, sel)
        with self.open_input_file("mini.pdb") as fh:
            prot2 = atom.read_pdb(fh, m, sel)

        xyzs1 = core.XYZs(atom.get_leaves(prot1))
        xyzs2 = core.XYZs(atom.get_leaves(prot2))
        drmsd = atom.get_drmsd(xyzs1, xyzs2)
        # Molecule with itself
        self.assertAlmostEqual(drmsd, 0)
        R = IMP.algebra.get_random_rotation_3d()
        v = IMP.algebra.get_random_vector_in(
            IMP.algebra.get_unit_bounding_box_3d())
        T = IMP.algebra.Transformation3D(R, v)
        for x in xyzs2:
            core.transform(x, T)
        drmsd = atom.get_drmsd(xyzs1, xyzs2)
        # Same thing after transformation
        self.assertAlmostEqual(drmsd, 0)
        #
        for x in xyzs2:
            R = IMP.algebra.get_random_rotation_3d()
            T = IMP.algebra.Transformation3D(R, v)
            core.transform(x, T)
        # test that the function is correctly implemented
        drmsd = 0.
        npairs = 0.
        for i in range(0, len(xyzs1) - 1):
            for j in range(i + 1, len(xyzs2)):
                dist0 = IMP.core.get_distance(xyzs1[i], xyzs1[j])
                dist1 = IMP.core.get_distance(xyzs2[i], xyzs2[j])
                drmsd += (dist0 - dist1) ** 2
                npairs += 1.
        drmsd1 = math.sqrt(drmsd / npairs)
        drmsd2 = atom.get_drmsd(xyzs1, xyzs2)
        self.assertAlmostEqual(drmsd1, drmsd2)

    def test__rigid_bodies_drmsd_Q(self):
        """ Test drmsd_Q measure"""
        m = IMP.Model()
        sel = atom.CAlphaPDBSelector()
        with self.open_input_file("mini.pdb") as fh:
            prot1 = atom.read_pdb(fh, m, sel)
        with self.open_input_file("mini.pdb") as fh:
            prot2 = atom.read_pdb(fh, m, sel)

        xyzs1 = core.XYZs(atom.get_leaves(prot1))
        xyzs2 = core.XYZs(atom.get_leaves(prot2))

        R = IMP.algebra.get_random_rotation_3d()
        v = IMP.algebra.get_random_vector_in(
            IMP.algebra.get_unit_bounding_box_3d())
        T = IMP.algebra.Transformation3D(R, v)
        for x in xyzs2:
            core.transform(x, T)

        thresholds = [10, 20, 30, 40, 60]
        for threshold in thresholds:

            #
            for x in xyzs2:
                R = IMP.algebra.get_random_rotation_3d()
                T = IMP.algebra.Transformation3D(R, v)
                core.transform(x, T)
            # test that the function is correctly implemented
            drmsd = 0.
            npairs = 0.
            for i in range(0, len(xyzs1) - 1):
                for j in range(i + 1, len(xyzs2)):
                    dist0 = IMP.core.get_distance(xyzs1[i], xyzs1[j])
                    dist1 = IMP.core.get_distance(xyzs2[i], xyzs2[j])
                    if dist0 <= threshold or dist1 <= threshold:
                        drmsd += (dist0 - dist1) ** 2
                        npairs += 1.
            drmsd = math.sqrt(drmsd / npairs)
            drmsd_target = atom.get_drmsd_Q(xyzs1, xyzs2, threshold)
            self.assertAlmostEqual(drmsd, drmsd_target)

        drmsd_Q = atom.get_drmsd_Q(xyzs1, xyzs2, 1000000.0)
        drmsd = atom.get_drmsd(xyzs1, xyzs2)
        self.assertAlmostEqual(drmsd, drmsd_Q)


if __name__ == '__main__':
    IMP.test.main()
