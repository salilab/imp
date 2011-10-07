import IMP
import IMP.test
import IMP.core as core
import IMP.atom as atom
import IMP.algebra as alg

class DistanceTest(IMP.test.TestCase):
    def test_placement_score(self):
        """Testing the mass and volume estimates"""
        m = IMP.Model()
        # read PDB
        mp= atom.read_pdb(self.open_input_file("input.pdb"),
                              m, atom.NonWaterPDBSelector())
        mp1= atom.read_pdb(self.open_input_file("input.pdb"),
                              m, atom.NonWaterPDBSelector())
        xyz=core.XYZs(atom.get_leaves(mp))
        xyz1=core.XYZs(atom.get_leaves(mp1))
        #create a random transformation
        t=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                       IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        for d in xyz1: core.transform(d,t)
        da=atom.get_placement_score(xyz1,xyz)
        d=t.get_translation().get_magnitude()
        a=IMP.algebra.get_axis_and_angle(t.get_rotation()).second
        self.assertAlmostEqual(da[0],d, 2)
        self.assertAlmostEqual(da[1],a, 2)


    def test_component_placement_score(self):
        """Testing that component placement score returns the same transformation if called twice"""
        m = IMP.Model()
        # read PDB
        mp1_ref= atom.read_pdb(self.open_input_file("1z5s_A.pdb"),
                              m, atom.NonWaterPDBSelector())
        mp1_mdl= atom.read_pdb(self.open_input_file("1z5s_A.pdb"),
                              m, atom.NonWaterPDBSelector())
        mp2_ref= atom.read_pdb(self.open_input_file("1z5s_C.pdb"),
                              m, atom.NonWaterPDBSelector())
        mp2_mdl= atom.read_pdb(self.open_input_file("1z5s_C.pdb"),
                              m, atom.NonWaterPDBSelector())
        xyz1_ref=core.XYZs(atom.get_leaves(mp1_ref))
        xyz1_mdl=core.XYZs(atom.get_leaves(mp1_mdl))
        xyz2_ref=core.XYZs(atom.get_leaves(mp2_ref))
        xyz2_mdl=core.XYZs(atom.get_leaves(mp2_mdl))

        #create a random transformation
        t=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                       IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        for d in xyz1_mdl: core.transform(d,t)
        t=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                       IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        #core.get_transformed(xyz2_mdl,t)
        for d in xyz2_mdl: core.transform(d, t)
        da1=atom.get_component_placement_score(xyz1_ref,xyz2_ref,xyz1_mdl,xyz2_mdl)
        da2=atom.get_component_placement_score(xyz1_ref,xyz2_ref,xyz1_mdl,xyz2_mdl)
        self.assertAlmostEqual(da1[1],da2[1])

    def test_drms(self):
        """ Test drms measure """
        m = IMP.Model()
        sel = atom.CAlphaPDBSelector()
        prot1 = atom.read_pdb(self.open_input_file("1DQK.pdb"), m, sel)
        prot2 = atom.read_pdb(self.open_input_file("1DQK.pdb"), m, sel)
        xyzs1 = core.XYZs(atom.get_leaves(prot1))
        xyzs2 = core.XYZs(atom.get_leaves(prot2))
        drms = atom.get_drms(xyzs1, xyzs2)
        # Molecule with itself
        self.assertAlmostEqual(drms, 0)
        R = IMP.algebra.get_random_rotation_3d()
        v = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        T = IMP.algebra.Transformation3D(R,v)
        for x in xyzs2:
            core.transform(x, T)
        drms = atom.get_drms(xyzs1, xyzs2)
        # Same thing after transformation
        self.assertAlmostEqual(drms, 0)
        #
        for x in xyzs2:
            R = IMP.algebra.get_random_rotation_3d()
            T = IMP.algebra.Transformation3D(R,v)
            core.transform(x, T)
        drms = atom.get_drms(xyzs1, xyzs2)
        self.assertTrue(drms > 0)

    def test__rigid_bodies_drms(self):
        """ Test drms measure taking into account rigid bodies"""
        m = IMP.Model()
        sel = atom.CAlphaPDBSelector()
        prot1 = atom.read_pdb(self.open_input_file("1DQK.pdb"), m, sel)
        prot2 = atom.read_pdb(self.open_input_file("1DQK.pdb"), m, sel)

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
        self.assertAlmostEqual(drms, rb_drms, delta=1e-3, msg="rb_drms != drms")
        # Same thing after transformation of each of the chains
        for h in hchains2:
            R = alg.get_random_rotation_3d()
            v = alg.get_random_vector_in(alg.get_unit_bounding_box_3d())
            T = alg.Transformation3D(R, v)
            ls = atom.get_leaves(h)
            for l in ls:
                core.transform(l.get_as_xyz(), T)
        drms = atom.get_drms(xyzs1, xyzs2)
        rb_drms = atom.get_rigid_bodies_drms(xyzs1, xyzs2, ranges)
        self.assertAlmostEqual(drms, rb_drms, delta=0.3, msg="rb_drms != drms")

if __name__ == '__main__':
    IMP.test.main()
