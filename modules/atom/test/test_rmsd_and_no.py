import IMP
import IMP.test
import IMP.core
import IMP.atom



class MeasuresTest(IMP.test.TestCase):
    def test_rmsd(self):
        """Testing proper results for RMSD"""
        m = IMP.Model()
        # read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        mp1= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        xyz=IMP.core.XYZs(IMP.atom.get_leaves(mp))
        xyz1=IMP.core.XYZs(IMP.atom.get_leaves(mp1))
        #create a random transformation
        rot=IMP.algebra.get_identity_rotation_3d()
        t=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                       IMP.algebra.Vector3D(5,0,0))
        for d in xyz1:
            IMP.core.transform(d,t)
        rmsd = IMP.atom.get_rmsd(xyz,xyz1)
        self.assertAlmostEqual(rmsd,5.0, 2)

        t=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                       IMP.algebra.Vector3D(0,5,5))
        for d in xyz1:
            IMP.core.transform(d,t)
        rmsd = IMP.atom.get_rmsd(xyz,xyz1)
        self.assertAlmostEqual(rmsd,(5**2+5**2+5**2)**0.5, 2)

    def test_native_overlap(self):
        """Testing proper results for native overlap"""
        m = IMP.Model()
        # read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        mp1= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        xyz=IMP.core.XYZs(IMP.atom.get_leaves(mp))
        xyz1=IMP.core.XYZs(IMP.atom.get_leaves(mp1))
        #create a random transformation
        rot=IMP.algebra.get_identity_rotation_3d()
        t=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                       IMP.algebra.Vector3D(5,0,0))
        for d in xyz1:
            IMP.core.transform(d,t)
        nat_ove = IMP.atom.get_native_overlap(xyz,xyz1,3.5)
        self.assertAlmostEqual(nat_ove,0, 2)
        nat_ove = IMP.atom.get_native_overlap(xyz,xyz1,6)
        self.assertAlmostEqual(nat_ove,100, 2)

    def test_fast_rmsd(self):
        """Testing proper results for RMSD"""
        m = IMP.Model()
        # read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                               m, IMP.atom.CAlphaPDBSelector())
        mp1= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.CAlphaPDBSelector())
        xyz=IMP.core.XYZs(IMP.atom.get_leaves(mp))
        xyz1=IMP.core.XYZs(IMP.atom.get_leaves(mp1))
        #create 10 random transformation
        ts=[]
        for i in range(10):
            rot=IMP.algebra.get_random_rotation_3d()
            ts.append(IMP.algebra.Transformation3D(rot,
                                                   IMP.algebra.Vector3D(5,0,0)))
        fast_cc_calc=IMP.atom.RMSDCalculator(IMP.atom.get_leaves(mp))
        #test that fast rmsd works like the regular rmsd
        id_t=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                          IMP.algebra.Vector3D(0,0,0))
        for t in ts:
            for d in xyz1:
                IMP.core.transform(d,t)
            rmsd1 = IMP.atom.get_rmsd(xyz,xyz1)
            rmsd2 = fast_cc_calc.get_rmsd(id_t,t)
            print rmsd1,rmsd2
            for d in xyz1:
                IMP.core.transform(d,t.get_inverse())
            self.assertAlmostEqual(rmsd1,rmsd2,2)

if __name__ == '__main__':
    IMP.test.main()
