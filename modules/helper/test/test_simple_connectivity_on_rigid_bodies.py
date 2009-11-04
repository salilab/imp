import unittest
import os
import IMP
import IMP.test
import IMP.atom
import IMP.helper

class SimpleConnectivityTest(IMP.test.TestCase):
    """Class to test simple connectivity restraint on rigid bodies"""

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)

        self.m = IMP.Model()
        sel = IMP.atom.CAlphaSelector()

        mhs = IMP.atom.Hierarchies()
        mh1 = IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                              self.m, sel)
        mh2 = IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                              self.m, sel)

        mhs.append(mh1)
        mhs.append(mh2)

        self.rbs = IMP.helper.set_rigid_bodies(mhs)

    def test_methods(self):
        """Check SimpleConnectivity's methods for rigid bodies"""

        sc = IMP.helper.create_simple_connectivity_on_rigid_bodies(self.rbs)
        r = sc.get_restraint()
        h = sc.get_harmonic_upper_bound()
        ps = sc.get_sphere_distance_pair_score()

        sc.set_mean(10.0)
        self.assertInTolerance (h.get_mean(), 10.0, 1e-4)

        sc.set_stddev(3.5)
        self.assertInTolerance (h.get_k(),
                                h.k_from_standard_deviation(3.5), 1e-4)
        sc.set_k(0.1)
        self.assertInTolerance (h.get_k(), 0.1, 1e-4)

        r.set_was_owned(True)
        r.show()

        self.m.evaluate(False)


if __name__ == '__main__':
    unittest.main()
