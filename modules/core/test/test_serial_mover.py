import IMP
import IMP.test
import IMP.core
import IMP.algebra
import pickle


class Test(IMP.test.TestCase):

    def make_system(self):
        self.m = IMP.Model()
        self.ps = []
        self.mvs = []
        for i in range(10):
            p = IMP.Particle(self.m)
            self.ps.append(p)
            IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(0, 0, 0))
            IMP.core.XYZ(p).set_coordinates_are_optimized(True)
            self.mvs.append(IMP.core.BallMover(self.m, p, 1.))
            self.mvs[-1].set_was_used(True)

    def test_pickle(self):
        """Test (un-)pickle of SerialMover"""
        self.make_system()
        mvr = IMP.core.SerialMover(self.mvs)
        mvr.set_name("foo")
        dump = pickle.dumps(mvr)

        newmvr = pickle.loads(dump)
        self.assertEqual(newmvr.get_name(), "foo")
        self.assertEqual(len(newmvr.get_movers()), 10)


if __name__ == '__main__':
    IMP.test.main()
