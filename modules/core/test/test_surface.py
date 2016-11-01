import IMP
import IMP.algebra
import IMP.core
import IMP.atom
import IMP.test


class Tests(IMP.test.TestCase):

    """Tests for Surface."""

    def test_init(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        surf = IMP.core.Surface.setup_particle(p)
        self.assertTrue(IMP.core.Surface.get_is_setup(m, p.get_index()))
        self.assertAlmostEqual(surf.get_coordinates().get_magnitude(), 0.)
        self.assertAlmostEqual((IMP.algebra.Vector3D(0, 0, 1) -
                                surf.get_normal()).get_magnitude(), 0.)

    def test_init_with_vectors(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        v = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
        n = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
        surf = IMP.core.Surface.setup_particle(p, v, n)
        self.assertTrue(IMP.core.Surface.get_is_setup(m, p.get_index()))
        self.assertAlmostEqual((v - surf.get_coordinates()).get_magnitude(),
                               0.)
        self.assertAlmostEqual((n.get_unit_vector() -
                                surf.get_normal()).get_magnitude(), 0.)

    def test_init_with_reference_frame(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        tr = IMP.algebra.get_random_local_transformation((1, 2, 3), 10., 3.)
        rf = IMP.algebra.ReferenceFrame3D(tr)
        surf = IMP.core.Surface.setup_particle(p, rf)
        self.assertTrue(IMP.core.Surface.get_is_setup(m, p.get_index()))
        norm = surf.get_normal()
        rot = IMP.algebra.get_transformation_from_first_to_second(
            IMP.algebra.ReferenceFrame3D(), rf).get_rotation()
        expect_norm = rot.get_rotated(
            IMP.algebra.Vector3D(0, 0, 1)).get_unit_vector()
        self.assertAlmostEqual((norm - expect_norm).get_magnitude(), 0.)
        self.assertAlmostEqual((rf.get_global_coordinates((0, 0, 0)) -
                               surf.get_coordinates()).get_magnitude(), 0)

    def test_dist_to_center(self):
        m = IMP.Model()
        for i in range(100):
            p = IMP.Particle(m)
            v0 = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
            n = IMP.algebra.get_random_vector_on_unit_sphere()
            surf = IMP.core.Surface.setup_particle(p, v0, n)
            v1 = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v1)
            xyzr = IMP.core.XYZR.setup_particle(xyz)
            xyzr.set_radius(1.)
            dist = IMP.algebra.get_distance(v0, v1)
            self.assertAlmostEqual(surf.get_distance_to_center(v1), dist)
            self.assertAlmostEqual(
                IMP.core.get_distance(IMP.core.XYZ(surf), xyz), dist)

    def test_distance(self):
        m = IMP.Model()
        for i in range(100):
            p = IMP.Particle(m)
            v0 = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
            n = IMP.algebra.get_random_vector_on_unit_sphere()
            surf = IMP.core.Surface.setup_particle(p, v0, n)
            v1 = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v1)
            xyzr = IMP.core.XYZR.setup_particle(xyz)
            xyzr.set_radius(1.)
            dist = abs(v1 * n - v0 * n)
            sphere_dist = dist - 1.
            self.assertAlmostEqual(surf.get_distance_to(v1), dist)
            self.assertAlmostEqual(
                IMP.core.get_distance(surf, xyz), dist)
            self.assertAlmostEqual(IMP.core.get_distance(surf, xyzr),
                                   sphere_dist)

    def test_reflect(self):
        m = IMP.Model()
        s = IMP.core.Surface.setup_particle(IMP.Particle(m))
        s.reflect()
        self.assertAlmostEqual((s.get_normal() -
                                IMP.algebra.Vector3D(0, 0, -1)
                                ).get_magnitude(), 0.)

    def test_height(self):
        m = IMP.Model()
        for i in range(100):
            p = IMP.Particle(m)
            v0 = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
            n = IMP.algebra.get_random_vector_on_unit_sphere()
            surf = IMP.core.Surface.setup_particle(p, v0, n)
            v1 = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
            point_height = v1 * n - v0 * n
            self.assertAlmostEqual(surf.get_height(v1), point_height)
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v1)
            xyzr = IMP.core.XYZR.setup_particle(xyz)
            xyzr.set_radius(1.)
            self.assertAlmostEqual(IMP.core.get_height(surf, xyz),
                                   point_height)
            sphere_height = point_height - 1.
            self.assertAlmostEqual(IMP.core.get_height(surf, xyzr),
                                   sphere_height)

    def test_depth(self):
        m = IMP.Model()
        for i in range(100):
            p = IMP.Particle(m)
            v0 = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
            n = IMP.algebra.get_random_vector_on_unit_sphere()
            surf = IMP.core.Surface.setup_particle(p, v0, n)
            v1 = 10 * IMP.algebra.get_random_vector_on_unit_sphere()
            point_depth = v0 * n - v1 * n
            self.assertAlmostEqual(surf.get_depth(v1), point_depth)
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v1)
            xyzr = IMP.core.XYZR.setup_particle(xyz)
            xyzr.set_radius(1.)
            self.assertAlmostEqual(IMP.core.get_depth(surf, xyz),
                                   point_depth)
            sphere_depth = point_depth - 1.
            self.assertAlmostEqual(IMP.core.get_depth(surf, xyzr),
                                   sphere_depth)

    def test_add_derivatives(self):
        m = IMP.Model()
        for i in range(10):
            s = IMP.core.Surface.setup_particle(IMP.Particle(m))
            v = IMP.algebra.get_random_vector_on_unit_sphere()
            s.add_to_normal_derivatives(v, IMP.DerivativeAccumulator())
            self.assertAlmostEqual(
                (s.get_normal_derivatives() - v).get_magnitude(), 0.)
            for i in range(3):
                self.assertAlmostEqual(s.get_normal_derivative(i), v[i])

    def test_surface_geometry_init(self):
        m = IMP.Model()
        surf = IMP.core.Surface.setup_particle(IMP.Particle(m))
        geo = IMP.core.get_constrained_surface_geometry(surf)
        geo.set_was_used(True)

    def test_lateral_surface_constraint(self):
        m = IMP.Model()
        s = IMP.core.Surface.setup_particle(IMP.Particle(m))
        d = IMP.core.XYZ.setup_particle(IMP.Particle(m))
        const = IMP.core.LateralSurfaceConstraint(s, d)
        const.set_was_used(True)
        for i in range(10):
            vc = IMP.algebra.get_random_vector_on_unit_sphere()
            s.set_coordinates(vc)
            vn = IMP.algebra.get_random_vector_on_unit_sphere()
            s.set_normal(vn)
            vd = IMP.algebra.get_random_vector_on_unit_sphere()
            d.set_coordinates(vd)
            new_center = s.get_plane().get_projected(vd)
            m.update()
            self.assertAlmostEqual(
                (s.get_coordinates() - new_center).get_magnitude(), 0.)
            self.assertAlmostEqual((s.get_normal() - vn).get_magnitude(), 0.)


if __name__ == '__main__':
    IMP.test.main()
