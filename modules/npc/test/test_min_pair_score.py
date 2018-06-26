import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.npc

class Tests(IMP.test.TestCase):

    def get_score(self, m, transforms):
        p0 = IMP.Particle(m, "p0")
        IMP.core.XYZR.setup_particle(p0, IMP.algebra.Sphere3D(
                                          IMP.algebra.Vector3D(2,0,0.), 0.4))
        p1 = IMP.Particle(m, "p1")
        IMP.core.XYZR.setup_particle(p1, IMP.algebra.Sphere3D(
                                          IMP.algebra.Vector3D(9,0,0.), 0.1))

        ps = IMP.npc.MinimumSphereDistancePairScore(IMP.core.Linear(0.0, 1.0),
                                                    transforms)
        r = IMP.core.PairRestraint(m, ps, [p0,p1])
        return r.evaluate(True)

    def test_no_transform(self):
        """Test pair score with no transforms"""
        m = IMP.Model()
        # Should return the regular sphere distance
        self.assertAlmostEqual(self.get_score(m, []), 6.5, delta=1e-6)

    def test_min_transform(self):
        """Test pair score with a transform that reduces the distance"""
        m = IMP.Model()
        transforms = [IMP.algebra.Transformation3D(
                          IMP.algebra.get_identity_rotation_3d(),
                          IMP.algebra.Vector3D(-10,0,0))]
        # This transform should move p1 to be closer to p0
        self.assertAlmostEqual(self.get_score(m, transforms), 2.5, delta=1e-6)

    def test_bad_transform(self):
        """Test pair score with a transform that doesn't reduce the distance"""
        m = IMP.Model()
        transforms = [IMP.algebra.Transformation3D(
                          IMP.algebra.get_identity_rotation_3d(),
                          IMP.algebra.Vector3D(10,0,0))]
        self.assertAlmostEqual(self.get_score(m, transforms), 6.5, delta=1e-6)

if __name__ == '__main__':
    IMP.test.main()
