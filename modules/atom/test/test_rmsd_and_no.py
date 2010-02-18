import unittest
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
        rot=IMP.algebra.identity_rotation ()
        t=IMP.algebra.Transformation3D(IMP.algebra.identity_rotation(),
                                       IMP.algebra.Vector3D(5,0,0))
        for d in xyz1:
            IMP.core.transform(d,t)
        rmsd = IMP.atom.rmsd(xyz,xyz1)
        self.assertAlmostEqual(rmsd,5.0, 2)

        t=IMP.algebra.Transformation3D(IMP.algebra.identity_rotation(),
                                       IMP.algebra.Vector3D(0,5,5))
        for d in xyz1:
            IMP.core.transform(d,t)
        rmsd = IMP.atom.rmsd(xyz,xyz1)
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
        rot=IMP.algebra.identity_rotation ()
        t=IMP.algebra.Transformation3D(IMP.algebra.identity_rotation(),
                                       IMP.algebra.Vector3D(5,0,0))
        for d in xyz1:
            IMP.core.transform(d,t)
        nat_ove = IMP.atom.native_overlap(xyz,xyz1,3.5)
        self.assertAlmostEqual(nat_ove,0, 2)
        nat_ove = IMP.atom.native_overlap(xyz,xyz1,6)
        self.assertAlmostEqual(nat_ove,100, 2)

if __name__ == '__main__':
    unittest.main()
