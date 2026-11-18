import IMP
import IMP.test
import IMP.em
import IMP.algebra

V = IMP.algebra.Vector3D


class Tests(IMP.test.TestCase):

    def test_outside_point(self):
        """Points outside of the map should give -FLT_MAX"""
        m = IMP.em.create_density_map(6, 6, 6, 1.0)
        m.reset_data(1.0)
        mdt = IMP.em.MapDistanceTransform(m, 0.5, 3.0)
        for pt in (V(100, 100, 100), V(-0.6, 3, 3), V(5.6, 3, 3),
                   V(3, -0.6, 3), V(3, 5.6, 3), V(3, 3, 5.6)):
            self.assertLess(mdt.get_distance_from_envelope(pt), -1e30)
        # Points inside the map should give a sensible value
        self.assertGreater(mdt.get_distance_from_envelope(V(3, 3, 3)), -1e30)


if __name__ == '__main__':
    IMP.test.main()
