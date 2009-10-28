import unittest
import os
import IMP
import IMP.test
import IMP.atom
import IMP.helper

class SimpleConnectivityTests(IMP.test.TestCase):
    """Tests for the SimpleConnectivity """

    def test_connectivity(self):

        def create_particles(m, coordinates):
            ps = IMP.Particles()
            radkey = IMP.FloatKey("radius")
            for pt in coordinates:
                p = self.create_point_particle(m, *pt)
                p.add_attribute(radkey, 1.0, False)
                ps.append(p)
            return ps

        def compute_and_print_distances(ps):
            distances = list()
            for i, p0 in enumerate(ps):
                print "*****************Distance %d******************" % i
                for p1 in ps:
                    if p1 == p0:
                        continue
                    dist = self.particle_distance(p0, p1)
                    print dist
                    distances.append(dist)
            print "*******************************************"
            return distances

        def check_closer_distances(old, new):
            for od, nd in zip(old, new):
                self.assert_(od > nd)

        IMP.set_log_level(IMP.VERBOSE)

        m = IMP.Model()
        ps = create_particles(m, [(0.0, 0.0, 0.0), (100.0, 0.0, 0.0), (0.0, 100.0, 0.0),
          (0.0, 0.0, 100.0)])

        old_dist = compute_and_print_distances(ps)

        sc = IMP.helper.create_simple_connectivity_on_molecules(ps)

        h = sc.harmonic_upper_bound()
        r = sc.restraint()

        sc.set_mean(0)
        self.assertEquals(h.get_mean(), 0)

        sc.set_k(1.5)
        self.assertInTolerance(h.get_k(), 1.5, 1e-4)

        sc.set_stddev(3.5)
        self.assertInTolerance(h.get_k(), h.k_from_standard_deviation(3.5), 1e-4)

        o = IMP.core.ConjugateGradients()
        o.set_threshold(1e-4)
        o.set_model(m)
        o.optimize(1000)

        new_dist = compute_and_print_distances(ps)

        check_closer_distances(old_dist, new_dist)

        r.show()
        m.evaluate(False)

        pps = r.get_connected_pairs()


if __name__ == '__main__':
    unittest.main()
