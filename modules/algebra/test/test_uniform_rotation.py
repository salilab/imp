import IMP
import IMP.test
import IMP.algebra
import random
import math

class Tests(IMP.test.TestCase):
    """Test rotations"""
    def test_euler_angles(self):
        """test conversion of euler angles"""
        n_tests = 1000
        print "Testing %d times" % n_tests
        for i in range(n_tests):
            phi=math.radians(random.randint(0,360))
            theta=math.radians(random.randint(0,180))
            psi=math.radians(random.randint(0,360))
            rot=IMP.algebra.get_rotation_from_fixed_zxz(phi,theta,psi)
            eu_angles=IMP.algebra.get_fixed_zxz_from_rotation(rot)
            phi1=eu_angles.get_phi()
            psi1=eu_angles.get_psi()
            theta1=eu_angles.get_theta()
            rot1=IMP.algebra.get_rotation_from_fixed_zxz(phi1,theta1,psi1)
            print "Test #%3d:" % i,
            print "phi %5.2f / %5.2f theta %5.2f / %5.2f psi %5.2f / %5.2f" \
                % (phi, phi1, theta, theta1, psi, psi1)
            self.assertAlmostEqual(IMP.algebra.get_distance(rot,rot1),0,3)
        print
    def test_euler_sampling(self):
        rots = IMP.algebra.get_uniformly_sampled_rotations(math.radians(30))
        print "number of rots",len(rots)
        self.assertEqual(552,len(rots))

if __name__ == '__main__':
    IMP.test.main()
