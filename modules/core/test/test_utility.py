import unittest
import IMP
import IMP.test
import IMP.atom
import IMP.core
import os
import time

class TestUtilities(IMP.test.TestCase):


    def test_transform(self):
        m = IMP.Model()
        q= IMP.algebra.random_vector_on_sphere(IMP.algebra.Vector3D(0.0,0.0,0.0),1.0)
        r= IMP.algebra.Rotation3D(q[0], q[1], q[2], 0.0)
        v= IMP.algebra.random_vector_in_box(IMP.algebra.Vector3D(-5.0,-5.0,-5.0),IMP.algebra.Vector3D(5.0,5.0,5.0))
        t = IMP.algebra.Transformation3D(r,v)
        mp= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                                        m, IMP.atom.NonWaterSelector())
        atoms = IMP.core.get_leaves(mp)
        c = IMP.core.centroid(atoms)
        IMP.core.transform(atoms,t)
        c_t = t.transform(c)
        self.assertAlmostEqual(IMP.algebra.distance(c_t,IMP.core.centroid(atoms)),0.0,1)

if __name__ == '__main__':
    unittest.main()
