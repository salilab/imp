import unittest
import IMP
import IMP.test
import IMP.core

class YamlTests(IMP.test.TestCase):
    def _create_model(self):
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        p0= IMP.Particle(m)
        d0=IMP.core.XYZ.create(p0, IMP.algebra.Vector3D(0,1,2))
        p1= IMP.Particle(m)
        d1=IMP.core.XYZ.create(p1, IMP.algebra.Vector3D(3,4,5))
        d0.set_coordinates_are_optimized(True)
        d1.set_coordinates_are_optimized(True)
        return (m,d0, d1)
    def test_yaml(self):
        """Check reading and writing optimized attributes of models """
        (m,d0, d1)= self._create_model()
        IMP.set_log_level(IMP.VERBOSE)
        IMP.core.write_optimized_attributes(m, open(self.get_tmp_file_name("test2.yaml"),
                                                    "w"))
        d0.set_coordinates(IMP.algebra.Vector3D(0,-1,-2))
        d1.set_coordinates(IMP.algebra.Vector3D(-3,-4,-5))
        d1.get_particle().set_is_optimized(IMP.core.XYZ.get_xyz_keys()[1], False)
        IMP.core.read_optimized_attributes(open(self.get_tmp_file_name("test2.yaml"), "r"), m)
        self.assertEqual(d0.get_coordinates()[0], 0)
        self.assertEqual(d0.get_coordinates()[1], 1)
        self.assertEqual(d0.get_coordinates()[2], 2)
        self.assertEqual(d1.get_coordinates()[0], 3)
        self.assertEqual(d1.get_coordinates()[1], -4)
        self.assertEqual(d1.get_coordinates()[2], 5)




if __name__ == '__main__':
    unittest.main()
