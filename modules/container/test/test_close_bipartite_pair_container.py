from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.container
import pickle


class Tests(IMP.test.TestCase):

    def make_system(self):
        m = IMP.Model()
        ps = IMP.get_indexes(self.create_particles_in_box(m, 6))
        for p in ps:
            IMP.core.XYZR.setup_particle(m, p, 0)
        pc1 = IMP.container.ListSingletonContainer(m, ps[:3])
        pc2 = IMP.container.ListSingletonContainer(m, ps[3:])
        cpss = IMP.container.CloseBipartitePairContainer(pc1, pc2, 3., 100.)
        return m, cpss

    def test_pickle(self):
        """Test (un-)pickle of CloseBipartitePairContainer"""
        m, cpss = self.make_system()
        cpss.set_name("foo")
        dump = pickle.dumps(cpss)

        newcpss = pickle.loads(dump)
        self.assertEqual(newcpss.get_name(), "foo")
        m.update()
        # should contain all a-b interactions, since slack is large
        self.assertEqual(len(newcpss.get_contents()), 3 * 3)


if __name__ == '__main__':
    IMP.test.main()
