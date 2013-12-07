import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra


class Tests(IMP.test.TestCase):

    def test_rmsd(self):
        """Testing proper results for RMSD"""
        m = IMP.kernel.Model()
        # read PDB
        mp = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                               m, IMP.atom.NonWaterPDBSelector())
        mp1 = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                                m, IMP.atom.NonWaterPDBSelector())
        xyz = IMP.core.XYZs(IMP.atom.get_leaves(mp))
        xyz1 = IMP.core.XYZs(IMP.atom.get_leaves(mp1))
        # create a random transformation
        rot = IMP.algebra.get_identity_rotation_3d()
        t = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.Vector3D(5, 0, 0))
        for d in xyz1:
            IMP.core.transform(d, t)
        rmsd = IMP.atom.get_rmsd(xyz, xyz1)
        self.assertAlmostEqual(rmsd, 5.0, 2)

        t = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.Vector3D(0, 5, 5))
        for d in xyz1:
            IMP.core.transform(d, t)
        rmsd = IMP.atom.get_rmsd(xyz, xyz1)
        self.assertAlmostEqual(rmsd, (5 ** 2 + 5 ** 2 + 5 ** 2) ** 0.5, 2)

    def test_simple_rmsd(self):
        """Test RMSD for vectors and XYZs"""
        v1 = IMP.algebra.Vector3D(0, 0, 0)
        v2 = IMP.algebra.Vector3D(1, 0, 0)

        m = IMP.kernel.Model()
        p1 = IMP.kernel.Particle(m)
        p2 = IMP.kernel.Particle(m)
        x1 = IMP.core.XYZ.setup_particle(p1, v1)
        x2 = IMP.core.XYZ.setup_particle(p2, v2)

        self.assertAlmostEqual(IMP.atom.get_rmsd([v1], [v2]), 1.0, delta=1e-6)
        self.assertAlmostEqual(IMP.atom.get_rmsd([x1], [x2]), 1.0, delta=1e-6)

    def test_native_overlap(self):
        """Testing proper results for native overlap"""
        m = IMP.kernel.Model()
        # read PDB
        mp = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                               m, IMP.atom.NonWaterPDBSelector())
        mp1 = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                                m, IMP.atom.NonWaterPDBSelector())
        xyz = IMP.core.XYZs(IMP.atom.get_leaves(mp))
        xyz1 = IMP.core.XYZs(IMP.atom.get_leaves(mp1))
        # create a random transformation
        rot = IMP.algebra.get_identity_rotation_3d()
        t = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.Vector3D(5, 0, 0))
        for d in xyz1:
            IMP.core.transform(d, t)
        nat_ove = IMP.atom.get_native_overlap(xyz, xyz1, 3.5)
        self.assertAlmostEqual(nat_ove, 0, 2)
        nat_ove = IMP.atom.get_native_overlap(xyz, xyz1, 6)
        self.assertAlmostEqual(nat_ove, 100, 2)

if __name__ == '__main__':
    IMP.test.main()
