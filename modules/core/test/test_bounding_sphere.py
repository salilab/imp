import IMP
import IMP.test
import IMP.algebra
import IMP.core


class Tests(IMP.test.TestCase):

    """Test BoundingSphere3DSingletonScore"""

    def test_getters(self):
        """Test BoundingSphere3DSingletonScore getter methods"""
        sphere = IMP.algebra.Sphere3D(
            IMP.algebra.Vector3D(1, 2, 3), 4)
        s = IMP.core.BoundingSphere3DSingletonScore(IMP.core.Harmonic(0, 1),
                                                    sphere)
        self.assertAlmostEqual(
            s.get_unary_function().get_derived_object().get_k(),
            1.0, delta=1e-4)
        self.assertLess(
            IMP.algebra.get_distance(s.get_sphere().get_center(),
                                     sphere.get_center()), 0.1)

    def _test_sphere_(self, sphere):
        """Test the box score of a random particle for a specific sphere"""
        self.d.set_coordinates(IMP.algebra.get_random_vector_in(self.bso))
#        print(self.d.get_coordinates())
        s = IMP.core.BoundingSphere3DSingletonScore(IMP.core.Harmonic(0, 1), sphere)
        r = IMP.core.SingletonRestraint(self.m, s, self.p)
        sf = IMP.core.RestraintsScoringFunction([r])
        o = IMP.core.ConjugateGradients(self.m)
        o.set_scoring_function(sf)
        o.optimize(100)
        for i in range(0, 3):
            v_from_origin= self.d.get_coordinates()-sphere.get_center()
            print(self.d.get_coordinates(),
                  sphere.get_center(),
                  v_from_origin.get_magnitude())
            threshold= self.R + 0.1
            if IMP.core.XYZR.get_is_setup(self.p):
                threshold= threshold - IMP.core.XYZR(self.p).get_radius()
            self.assertLess(v_from_origin.get_magnitude(), threshold)

    def test_spheres(self):
        self.R=10
        self.m = IMP.Model()
        self.p = IMP.Particle(self.m)
        self.d = IMP.core.XYZ.setup_particle(self.p)
        self.d.set_coordinates_are_optimized(True)
        self.bso= IMP.algebra.Sphere3D([0,0,0],30)
        bsi= IMP.algebra.Sphere3D([0,0,0], self.R)
        print("Bounding sphere I for xyz")
        for i in range(10):
            self._test_sphere_(bsi)
        print("Bounding sphere II for xyz")
        bsii= IMP.algebra.Sphere3D([50,50,50],self.R)
        for i in range(10):
            self._test_sphere_(bsii)
        IMP.core.XYZR.setup_particle(self.p, 2.0)
        print("Bounding sphere I for xyzr")
        for i in range(10):
            self._test_sphere_(bsi)
        print("Bounding sphere II for xyzr")
        bsii= IMP.algebra.Sphere3D([50,50,50],self.R)
        for i in range(10):
            self._test_sphere_(bsii)



if __name__ == '__main__':
    IMP.test.main()
