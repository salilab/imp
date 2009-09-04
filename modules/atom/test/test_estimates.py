import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom



class MCOptimizerTest(IMP.test.TestCase):
    def test_massvol(self):
        """Testing the mass and volume estimates"""
        m= IMP.atom.mass_from_number_of_residues(250)
        print m
        v= IMP.atom.volume_from_mass(m)
        print v
        r= (v/(4.0*3.1415)*3.0)**.333
        rc= 0.726*m**.333 # from Frank's paper
        print r
        print rc
        self.assertInTolerance(r, rc, .1*r)

if __name__ == '__main__':
    unittest.main()
