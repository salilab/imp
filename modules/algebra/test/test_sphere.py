from __future__ import print_function
import IMP
import IMP.test
import IMP.algebra
import math
import io
import pickle


class Tests(IMP.test.TestCase):

    def test_sphere_construction(self):
        """Check that sphere are constructed correctly"""
        center = IMP.algebra.Vector3D(0.0, 0.0, 0.0)
        radius = 5.0
        sph = IMP.algebra.Sphere3D(center, radius)
        self.assertEqual(
            (sph.get_center() - center).get_magnitude() < 0.01,
            True)
        self.assertEqual(sph.get_radius(), radius)
        self.assertAlmostEqual(
            IMP.algebra.get_surface_area(sph), math.pi * 25.0 * 4,
            places=1)
        self.assertAlmostEqual(
            IMP.algebra.get_volume(sph), math.pi * 125.0 * (4.0 / 3),
            places=1)

    def test_random_vector_2d(self):
        """Test random vectors on a 2D sphere (circle)"""
        s = IMP.algebra.Sphere2D(IMP.algebra.Vector2D(1, 2), 3)
        v = IMP.algebra.get_random_vector_in(s)
        self.assertLess(IMP.algebra.get_distance(v, s.get_center()), 3. + 1e-6)

    def test_enclosing_sphere(self):
        """Check enclosing sphere of spheres"""
        ss = []
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(10, 10, 10))
        for i in range(0, 20):
            v = IMP.algebra.get_random_vector_in(bb)
            r = i
            s = IMP.algebra.Sphere3D(v, r)
            ss.append(s)
        for i in range(1, 20):
            css = ss[0:i]
            print(len(css))
            es = IMP.algebra.get_enclosing_sphere(css)
            es.show()
            print()
            for cs in css:
                d = (cs.get_center() - es.get_center()).get_magnitude()
                cs.show()
                print()
                self.assertLess(d + cs.get_radius() - es.get_radius(), .5)

    def test_enclosing_vector(self):
        """Check enclosing sphere of vectors"""
        vs = []
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(10, 10, 10))
        for i in range(20):
            vs.append(IMP.algebra.get_random_vector_in(bb))
        for i in range(1, 20):
            vss = vs[:i]
            es = IMP.algebra.get_enclosing_sphere(vss)
            for v in vss:
                d = (v - es.get_center()).get_magnitude()
                self.assertLess(d - es.get_radius(), .5)

    def test_sampling_in_sphere(self):
        """Verify uniform sampling within a sphere"""
        s = IMP.algebra.Sphere2D([0, 0], 1)
        inner_box = \
            IMP.algebra.BoundingBox2D([-0.5, -0.5], [0.5, 0.5])
        n = 0
        m = 2000
        for i in range(m):
            v = IMP.algebra.get_random_vector_in(s)
            if inner_box.get_contains(v):
                n = n + 1
        # compare expected and observed hits
        s_area = math.pi  # pi * 1.0^2
        inner_box_area = 1.0
        expected_p = inner_box_area / s_area
        observed_p = float(n) / float(m)
        print(expected_p, observed_p)
        self.assertAlmostEqual(observed_p, expected_p, places=1)

    def test_io(self):
        """Check I/O of Sphere3Ds"""
        V = IMP.algebra.Vector3D
        S = IMP.algebra.Sphere3D
        vs1 = [S(V(1, 2, 3), 4),
               S(V(4, 5, 6), 7)]

        sio = io.BytesIO()
        IMP.algebra.write_spheres(vs1, sio)
        sio.seek(0)
        rpts = IMP.algebra.read_spheres(sio)
        self.assertEqual(len(rpts), len(vs1))
        for i in range(0, len(rpts)):
            self.assertAlmostEqual(rpts[i].get_radius(), vs1[i].get_radius(),
                                   delta=0.01)
            dist = IMP.algebra.get_distance(rpts[i].get_center(),
                                            vs1[i].get_center())
            self.assertLess(dist, 0.01)
        sio = io.BytesIO(b"garbage")
        self.assertRaises(ValueError, IMP.algebra.read_spheres, sio)

    def test_sphere_nd(self):
        """Test SphereD<N> operations for unusual N"""
        for N in (-1, 1, 2, 4, 5, 6):
            if N == -1:
                clsdim = 'K'
                dim = 5
            else:
                clsdim = '%d' % N
                dim = N
            V = getattr(IMP.algebra, "Vector%sD" % clsdim)
            S = getattr(IMP.algebra, "Sphere%sD" % clsdim)
            v = V([0] * dim)
            s = S(v, 1.0)
            self.assertLess(IMP.algebra.get_distance(s, s), 1e-4)
            if N == -1:
                us = IMP.algebra.get_unit_sphere_kd(5)
            else:
                us = getattr(IMP.algebra, "get_unit_sphere_%sd" % clsdim)()
            bb = IMP.algebra.get_bounding_box(s)
            self.assertTrue(IMP.algebra.get_interiors_intersect(s, s))
            self.assertTrue(s.get_contains(v))
            self.assertEqual(s.get_dimension(), dim)
            self.assertAlmostEqual(s.get_radius(), 1.0, delta=1e-4)
            self.assertLess(IMP.algebra.get_distance(s.get_center(), v), 1e-4)
            sio = io.BytesIO()
            s.show(sio)

    def test_pickle_3d(self):
        """Check (un-)pickle of Sphere3Ds"""
        v1 = IMP.algebra.Vector3D(3.0, 6.0, 9.0)
        v2 = IMP.algebra.Vector3D(1., 2., 3.)
        s1 = IMP.algebra.Sphere3D(v1, 4)
        s2 = IMP.algebra.Sphere3D(v2, 4)
        s2.foo = 'bar'
        sdump = pickle.dumps((s1, s2))

        news1, news2 = pickle.loads(sdump)

        self.assertAlmostEqual(s1.get_radius(), news1.get_radius(), delta=0.01)
        self.assertAlmostEqual(s2.get_radius(), news2.get_radius(), delta=0.01)
        self.assertLess(
            IMP.algebra.get_distance(s1.get_center(), news1.get_center()),
            1e-4)
        self.assertLess(
            IMP.algebra.get_distance(s2.get_center(), news2.get_center()),
            1e-4)
        self.assertEqual(news2.foo, 'bar')

    def test_pickle_kd(self):
        """Check (un-)pickle of SphereKDs"""
        v1 = IMP.algebra.VectorKD([3.0, 6.0, 9.0, 12.0])
        v2 = IMP.algebra.VectorKD([1., 2.])
        s1 = IMP.algebra.SphereKD(v1, 4)
        s2 = IMP.algebra.SphereKD(v2, 4)
        s2.foo = 'bar'
        sdump = pickle.dumps((s1, s2))

        news1, news2 = pickle.loads(sdump)

        self.assertAlmostEqual(s1.get_radius(), news1.get_radius(), delta=0.01)
        self.assertAlmostEqual(s2.get_radius(), news2.get_radius(), delta=0.01)
        self.assertLess(
            IMP.algebra.get_distance(s1.get_center(), news1.get_center()),
            1e-4)
        self.assertLess(
            IMP.algebra.get_distance(s2.get_center(), news2.get_center()),
            1e-4)
        self.assertEqual(news2.foo, 'bar')


if __name__ == '__main__':
    IMP.test.main()
