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

    def count_moved(self, ps):
        num_moved = 0
        for p in ps:
            if ((IMP.core.XYZ(p).get_coordinates() -
                 IMP.algebra.Vector3D(0, 0, 0)).get_magnitude() > 0):
                num_moved += 1
        return num_moved

    def test_propose_sample_1(self):
        """Propose a composite move with mover subset size 1"""
        self.make_system()
        subset_mvr = IMP.core.SubsetMover(self.mvs, 1)
        res = subset_mvr.propose()
        num_moved = self.count_moved(self.ps)
        self.assertEqual(num_moved, 1)
        self.assertEqual(len(res.get_moved_particles()), 1)

    def test_propose_sample_5(self):
        """Propose a composite move with mover subset size 5"""
        self.make_system()
        subset_mvr = IMP.core.SubsetMover(self.mvs, 5)
        res = subset_mvr.propose()
        num_moved = self.count_moved(self.ps)
        self.assertEqual(num_moved, 5)
        self.assertEqual(len(res.get_moved_particles()), 5)

    def test_propose_sample_all(self):
        """Propose a composite move with all movers."""
        self.make_system()
        subset_mvr = IMP.core.SubsetMover(self.mvs, 10)
        res = subset_mvr.propose()
        num_moved = self.count_moved(self.ps)
        self.assertEqual(num_moved, 10)
        self.assertEqual(len(res.get_moved_particles()), 10)

    def test_is_random(self):
        """Test all movers are used."""
        times_moved = [0 for i in range(10)]
        for i in range(100):
            self.make_system()
            subset_mvr = IMP.core.SubsetMover(self.mvs, 1)
            subset_mvr.propose()
            for j, p in enumerate(self.ps):
                if ((IMP.core.XYZ(p).get_coordinates() -
                     IMP.algebra.Vector3D(0, 0, 0)).get_magnitude() > 0):
                    times_moved[j] += 1
        self.assertTrue(all([x > 0 for x in times_moved]))

    def test_reject(self):
        """Test all movers are rejected."""
        self.make_system()
        subset_mvr = IMP.core.SubsetMover(self.mvs, 5)
        subset_mvr.propose()
        subset_mvr.reject()
        num_moved = self.count_moved(self.ps)
        self.assertEqual(num_moved, 0)

    def test_pickle(self):
        """Test (un-)pickle of SubsetMover"""
        self.make_system()
        mvr = IMP.core.SubsetMover(self.mvs, 5)
        mvr.set_name("foo")
        dump = pickle.dumps(mvr)

        newmvr = pickle.loads(dump)
        self.assertEqual(newmvr.get_name(), "foo")
        self.assertEqual(len(newmvr.get_movers()), 10)
        self.assertEqual(newmvr.get_subset_size(), 5)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of SubsetMover via polymorphic pointer"""
        self.make_system()
        mvr = IMP.core.SubsetMover(self.mvs, 5)
        mvr.set_name("foo")
        sm = IMP.core.SerialMover([mvr])
        dump = pickle.dumps(sm)

        newsm = pickle.loads(dump)
        newmvr, = newsm.get_movers()
        self.assertEqual(newmvr.get_name(), "foo")


if __name__ == '__main__':
    IMP.test.main()
