from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.algebra

class Tests(IMP.test.TestCase):
    def test_linear_restraint(self):
        m = IMP.Model()

        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)

        xyz1 = IMP.core.XYZ.setup_particle(p1)
        xyz2 = IMP.core.XYZ.setup_particle(p2)

        xyz1.set_coordinates((0, 0, 0))
        xyz2.set_coordinates((0, 0, 0))

        linear = IMP.core.Linear(0, 0.0)
        linear.set_slope(0.01)
        dps2 = IMP.core.DistancePairScore(linear)
        lr = IMP.core.PairRestraint(m, dps2, (p1, p2))
        lr.set_was_used(True)

        maxdist = 40.0
        npoints = 100

        dists = []
        scores = []
        for i in range(npoints):
            xyz2.set_coordinates(
                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0.0, 0.0))
            dist = IMP.core.get_distance(xyz1, xyz2)
            score = lr.unprotected_evaluate(None)
            print(dist, score)

if __name__ == '__main__':
    IMP.test.main()
