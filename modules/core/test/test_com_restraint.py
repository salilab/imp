import math
import random

import IMP
import IMP.algebra
import IMP.core
import IMP.test


def make_particles(m, coords):
    ps = []
    for c in coords:
        p = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(*c))
        ps.append(p)
    return ps


def weighted_centroid(ps, weights):
    total = sum(weights)
    c = IMP.algebra.Vector3D(0, 0, 0)
    for p, w in zip(ps, weights):
        c += IMP.core.XYZ(p).get_coordinates() * (w / total)
    return c


class Tests(IMP.test.TestCase):

    def test_weighted_centroid(self):
        """The COM is the weighted mean of the coordinates"""
        m = IMP.Model()
        ps = make_particles(m, [(0, 0, 0), (4, 0, 0)])
        r = IMP.core.COMRestraint(m)
        g = r.add_group([p.get_index() for p in ps], [1.0, 3.0])
        self.assertLess(IMP.algebra.get_distance(
            r.get_center_of_mass(g), IMP.algebra.Vector3D(3, 0, 0)), 1e-6)

    def test_equal_weights(self):
        """Without weights the COM is the centroid"""
        m = IMP.Model()
        ps = make_particles(m, [(0, 0, 0), (4, 0, 0), (2, 6, 0)])
        r = IMP.core.COMRestraint(m)
        g = r.add_group([p.get_index() for p in ps])
        self.assertLess(IMP.algebra.get_distance(
            r.get_center_of_mass(g), IMP.algebra.Vector3D(2, 2, 0)), 1e-6)

    def _single_term(self, kind):
        m = IMP.Model()
        ps = make_particles(m, [(1, 2, 3), (4, -1, 5), (2, 2, -2)])
        qs = make_particles(m, [(10, 1, 0), (12, 3, 1)])
        wp, wq = [1.0, 2.0, 3.0], [2.0, 1.0]
        r = IMP.core.COMRestraint(m)
        g = r.add_group([p.get_index() for p in ps], wp)
        h = r.add_group([p.get_index() for p in qs], wq)
        f = IMP.core.Harmonic(0.0, 1.0)  # 0.5 * feature^2
        point = IMP.algebra.Vector3D(1, 1, 1)
        direction = IMP.algebra.Vector3D(0, 1, 1)
        u = direction.get_unit_vector()

        def feature():
            c = weighted_centroid(ps, wp)
            if kind == 'point':
                return IMP.algebra.get_distance(c, point)
            elif kind == 'plane':
                return (c - point) * u
            elif kind == 'axis':
                d = c - point
                return (d - u * (d * u)).get_magnitude()
            else:
                return IMP.algebra.get_distance(c, weighted_centroid(qs, wq))

        if kind == 'point':
            r.add_point_term(g, point, f)
        elif kind == 'plane':
            r.add_plane_term(g, point, direction, f)
        elif kind == 'axis':
            r.add_axis_term(g, point, direction, f)
        else:
            r.add_pair_term(g, h, f)
        return m, r, ps + qs, feature

    def test_terms(self):
        """Each term scores its geometric quantity"""
        for kind in ('point', 'plane', 'axis', 'pair'):
            m, r, ps, feature = self._single_term(kind)
            self.assertAlmostEqual(r.evaluate(False), 0.5 * feature() ** 2,
                                   delta=1e-6, msg=kind)

    def test_derivatives(self):
        """Derivatives match finite differences"""
        for kind in ('point', 'plane', 'axis', 'pair'):
            m, r, ps, feature = self._single_term(kind)
            sf = IMP.core.RestraintsScoringFunction([r])
            sf.evaluate(True)
            for p in ps:
                d = IMP.core.XYZ(p).get_derivatives()
                for i in range(3):
                    x = IMP.core.XYZ(p)
                    c = x.get_coordinate(i)
                    h = 1e-4
                    x.set_coordinate(i, c + h)
                    up = sf.evaluate(False)
                    x.set_coordinate(i, c - h)
                    down = sf.evaluate(False)
                    x.set_coordinate(i, c)
                    self.assertAlmostEqual(d[i], (up - down) / (2 * h),
                                           delta=1e-3, msg=kind)

    def test_moved(self):
        """Scoring only moved particles gives the same score as a full
        evaluation, for beads, rigid bodies and unrelated particles"""
        m = IMP.Model()
        random.seed(3)
        IMP.random_number_generator.seed(3)

        def rand():
            return (random.uniform(-20, 20), random.uniform(-20, 20),
                    random.uniform(-20, 20))

        body = make_particles(m, [rand() for _ in range(5)])
        rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m), body)
        beads = make_particles(m, [rand() for _ in range(4)])
        other = make_particles(m, [rand()])[0]
        r = IMP.core.COMRestraint(m)
        g1 = r.add_group([p.get_index() for p in body[:3] + beads[:2]],
                         [1.0, 1.0, 1.0, 2.0, 2.0])
        g2 = r.add_group([p.get_index() for p in body[3:] + beads[2:]],
                         [1.0, 1.0, 2.0, 2.0])
        f = IMP.core.Harmonic(5.0, 1.0)
        r.add_point_term(g1, IMP.algebra.Vector3D(0, 0, 0), f)
        r.add_plane_term(g2, IMP.algebra.Vector3D(0, 0, 2),
                         IMP.algebra.Vector3D(0, 0, 1),
                         IMP.core.SoftLowerBound(3.0))
        r.add_axis_term(g1, IMP.algebra.Vector3D(1, 0, 0),
                        IMP.algebra.Vector3D(0, 0, 1), f)
        r.add_pair_term(g1, g2, f)
        sf = IMP.core.RestraintsScoringFunction([r])
        sf.evaluate(False)
        for step in range(60):
            choice = step % 3
            if choice == 0:
                IMP.core.transform(rb, IMP.algebra.Transformation3D(
                    IMP.algebra.get_random_rotation_3d(),
                    IMP.algebra.Vector3D(*rand()) * 0.1))
                moved = [rb.get_particle_index()]
            elif choice == 1:
                p = random.choice(beads)
                IMP.core.XYZ(p).set_coordinates(IMP.algebra.Vector3D(*rand()))
                moved = [p.get_index()]
            else:
                IMP.core.XYZ(other).set_coordinates(
                    IMP.algebra.Vector3D(*rand()))
                moved = [other.get_index()]
            got = sf.evaluate_moved(False, moved, [])
            fresh = IMP.core.RestraintsScoringFunction([r]).evaluate(False)
            self.assertAlmostEqual(got, fresh, delta=1e-6)

    def test_soft_lower_bound(self):
        """SoftLowerBound is log(1 + exp(-x / softness))"""
        f = IMP.core.SoftLowerBound(2.0)
        for x in (-50.0, -3.0, 0.0, 1.5, 40.0):
            self.assertAlmostEqual(f.evaluate(x),
                                   math.log1p(math.exp(-x / 2.0)), delta=1e-9)
            h = 1e-5
            num = (f.evaluate(x + h) - f.evaluate(x - h)) / (2 * h)
            self.assertAlmostEqual(f.evaluate_with_derivative(x)[1], num,
                                   delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
