from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.container
import pickle


class Tests(IMP.test.TestCase):

    def make_system(self):
        m = IMP.Model()
        ps = IMP.get_indexes(self.create_particles_in_box(m, 4))
        for p in ps:
            IMP.core.XYZR.setup_particle(m, p, 0)
        pc = IMP.container.ListSingletonContainer(m, ps)
        cpss = IMP.container.ClosePairContainer(
            pc, 1, IMP.core.GridClosePairsFinder(), 100.)
        return m, cpss

    def test_pickle(self):
        """Test (un-)pickle of ClosePairContainer"""
        m, cpss = self.make_system()
        cpss.set_name("foo")
        dump = pickle.dumps(cpss)

        newcpss = pickle.loads(dump)
        self.assertEqual(newcpss.get_name(), "foo")
        m.update()
        # should contain all interactions for 4 particles, since slack is large
        self.assertEqual(len(newcpss.get_contents()), 6)


if __name__ == '__main__':
    IMP.test.main()
