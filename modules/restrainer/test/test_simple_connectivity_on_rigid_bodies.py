import os
import IMP
import IMP.test
import IMP.atom
import IMP.restrainer

class Tests(IMP.test.TestCase):
    """Class to test simple connectivity restraint on rigid bodies"""

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)

        self.m = IMP.Model()
        sel = IMP.atom.CAlphaPDBSelector()

        mhs = IMP.atom.Hierarchies()
        mh1 = IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                              self.m, sel)
        IMP.atom.add_radii(mh1)
        mh2 = IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                              self.m, sel)
        IMP.atom.add_radii(mh2)

        mhs.append(mh1)
        mhs.append(mh2)

        self.rbs = IMP.restrainer.set_rigid_bodies(mhs)

    def test_methods(self):
        """Check SimpleConnectivity's methods for rigid bodies"""

        sc = IMP.restrainer.create_simple_connectivity_on_rigid_bodies(self.rbs)
        r = sc.get_restraint()
        self.m.add_restraint(r)

        h = sc.get_harmonic_upper_bound()
        ps = sc.get_sphere_distance_pair_score()

        sc.set_mean(10.0)
        self.assertAlmostEqual(h.get_mean(), 10.0, delta=1e-4)

        sc.set_standard_deviation(3.5)
        self.assertAlmostEqual(h.get_k(),
                               h.get_k_from_standard_deviation(3.5), delta=1e-4)
        sc.set_k(0.1)
        self.assertAlmostEqual(h.get_k(), 0.1, delta=1e-4)

        r.set_was_used(True)
        r.show()

        self.m.evaluate(False)


if __name__ == '__main__':
    IMP.test.main()
