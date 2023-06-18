import IMP
import IMP.test
import IMP.core
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of ConjugateGradients"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        d1 = IMP.core.XYZ.setup_particle(p1, IMP.algebra.Vector3D(0,0,0))
        d2 = IMP.core.XYZ.setup_particle(p2, IMP.algebra.Vector3D(1,1,1))
        d2.set_coordinates_are_optimized(True)
        dr = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(0, 1), p1, p2)
        cg = IMP.core.ConjugateGradients(m)
        sf = IMP.core.RestraintsScoringFunction([dr])
        cg.set_scoring_function(sf)
        cg.set_name("foo")
        dump = pickle.dumps(cg)
        newcg = pickle.loads(dump)
        self.assertEqual(newcg.get_name(), "foo")
        # CG should pull particles together
        newcg.optimize(20)
        dist = IMP.algebra.get_distance(
            d1.get_coordinates(), d2.get_coordinates())
        self.assertLess(dist, 0.1)


if __name__ == '__main__':
    IMP.test.main()
